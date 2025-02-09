// Copyright Epic Games, Inc. All Rights Reserved.

#include "lastProjectPlayerController.h"
#include "GameFramework/Pawn.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "lastProjectCharacter.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/Character.h"
#include "PlayerAnim.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Mine.h"
#include "Components/DecalComponent.h"
#include "BossFSM.h"
#include "Boss.h"
#include"CollisionShape.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/OverlapResult.h"


DEFINE_LOG_CATEGORY(LogTemplateCharacter);

AlastProjectPlayerController::AlastProjectPlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
	CachedDestination = FVector::ZeroVector;
	FollowTime = 0.f;
	bIsBombingActive = false;

	SpawnDistance = 200.0f;

	// �ʱⰪ ����
	bCanUseSniper = true;
	bCanUseMine = true;
	bCanUseBombing = true;
	bCanUseRoll = true;

	SniperCooldownTime = 2.0f;   // 2�� ��Ÿ��
	MineCooldownTime = 8.0f;    // 8�� ��Ÿ��
	BombingCooldownTime = 12.0f; // 12�� ��Ÿ��
	RollCooldownTime = 3.0f;
}

void AlastProjectPlayerController::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	if (BackgroundMusic)
	{
		BGMComponent = UGameplayStatics::CreateSound2D(this, BackgroundMusic);
		if (BGMComponent)
		{
			BGMComponent->Play();
			BGMComponent->SetVolumeMultiplier(0.5f); // ���� ���� (0.0 ~ 1.0)
		}
	}

	// Crosshair Widget ���� �� ȭ�鿡 �߰�
	if (CrosshairWidgetClass)
	{
		CrosshairWidget = CreateWidget<UUserWidget>(this, CrosshairWidgetClass);
		if (CrosshairWidget)
		{
			CrosshairWidget->AddToViewport();
			CrosshairWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	if (SkillCooldownWidgetClass)
	{
		SkillCooldownWidget = CreateWidget<USkillCooldownWidget>(this, SkillCooldownWidgetClass);
		if (SkillCooldownWidget)
		{
			SkillCooldownWidget->AddToViewport();
		}
	}

	if (playerHUDWidgetClass)
	{
		playerHUDWidget = CreateWidget<UPlayerHUDWidget>(this, playerHUDWidgetClass);
		if (playerHUDWidget)
		{
			playerHUDWidget->AddToViewport();
		}
	}
}

void AlastProjectPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Crosshair ��ġ ������Ʈ
	UpdateCrosshairPosition();

	if (bIsBombingActive && BombingRangeActor)
	{
		FVector WorldLocation, WorldDirection;
		if (DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
		{
			FVector TraceStart = WorldLocation;
			FVector TraceEnd = TraceStart + WorldDirection * 10000.0f;

			FHitResult HitResult;
			if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility))
			{
				BombingTargetLocation = HitResult.Location;
				BombingRangeActor->SetActorLocation(BombingTargetLocation);
			}
		}
	}
}


void AlastProjectPlayerController::SetupInputComponent()
{
	// set up gameplay key bindings
	Super::SetupInputComponent();

	// Add Input Mapping Context
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		// Setup mouse input events
		EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Started, this, &AlastProjectPlayerController::OnInputStarted);
		EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Triggered, this, &AlastProjectPlayerController::OnSetDestinationTriggered);
		EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Completed, this, &AlastProjectPlayerController::OnSetDestinationReleased);
		EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Canceled, this, &AlastProjectPlayerController::OnSetDestinationReleased);

		// Setup touch input events
		EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Started, this, &AlastProjectPlayerController::OnInputStarted);
		EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Triggered, this, &AlastProjectPlayerController::OnTouchTriggered);
		EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Completed, this, &AlastProjectPlayerController::OnTouchReleased);
		EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Canceled, this, &AlastProjectPlayerController::OnTouchReleased);
		EnhancedInputComponent->BindAction(IA_Roll, ETriggerEvent::Started, this, & AlastProjectPlayerController::Roll);
		
		EnhancedInputComponent->BindAction(IA_Fire, ETriggerEvent::Started, this, & AlastProjectPlayerController::Fire);
		
		EnhancedInputComponent->BindAction(IA_Sniper, ETriggerEvent::Started, this, & AlastProjectPlayerController::Sniper);
		
		EnhancedInputComponent->BindAction(IA_Gatling, ETriggerEvent::Started, this, & AlastProjectPlayerController::Gatling);
		
		EnhancedInputComponent->BindAction(IA_Mine, ETriggerEvent::Started, this, & AlastProjectPlayerController::SpawnMine);
		
		EnhancedInputComponent->BindAction(IA_Bombing, ETriggerEvent::Started, this, &AlastProjectPlayerController::StartBombing);

		EnhancedInputComponent->BindAction(IA_BombingActive, ETriggerEvent::Started, this, &AlastProjectPlayerController::ConfirmBombing);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AlastProjectPlayerController::OnInputStarted()
{
	StopMovement();
}

// Triggered every frame when the input is held down
void AlastProjectPlayerController::OnSetDestinationTriggered()
{
	if (isDie)
		return;

	if (isSniping || isMine) return;


	FollowTime += GetWorld()->GetDeltaSeconds();

	// We look for the location in the world where the player has pressed the input
	FHitResult Hit;
	bool bHitSuccessful = false;
	if (bIsTouch)
	{
		bHitSuccessful = GetHitResultUnderFinger(ETouchIndex::Touch1, ECollisionChannel::ECC_Visibility, true, Hit);
	}
	else
	{
		bHitSuccessful = GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, true, Hit);
	}

	// If we hit a surface, cache the location
	if (bHitSuccessful)
	{
		CachedDestination = Hit.Location;
	}
	
	// Move towards mouse pointer or touch
	APawn* ControlledPawn = GetPawn();
	if (ControlledPawn != nullptr)
	{
		FVector WorldDirection = (CachedDestination - ControlledPawn->GetActorLocation()).GetSafeNormal();
		ControlledPawn->AddMovementInput(WorldDirection, 1.0, false);
	}

	
}

void AlastProjectPlayerController::OnSetDestinationReleased()
{
	if (isDie)
		return;

	if (isSniping || isMine) return;

	if (FollowTime <= ShortPressThreshold)
	{
		// We move there and spawn some particles
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, CachedDestination);
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, FXCursor, CachedDestination, FRotator::ZeroRotator, FVector(1.f, 1.f, 1.f), true, true, ENCPoolMethod::None, true);
	}

	FollowTime = 0.f;
}

// Triggered every frame when the input is held down
void AlastProjectPlayerController::OnTouchTriggered()
{
	bIsTouch = true;
	OnSetDestinationTriggered();
}

void AlastProjectPlayerController::OnTouchReleased()
{
	bIsTouch = false;
	OnSetDestinationReleased();
}

void AlastProjectPlayerController::Roll()
{
	if (isDie)
		return;


	if (!bCanUseRoll)
		return;

	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn) return;

	ACharacter* ControlledCharacter = Cast<ACharacter>(ControlledPawn);
	if (!ControlledCharacter) return;

	UPlayerAnim* anim = Cast<UPlayerAnim>(ControlledCharacter->GetMesh()->GetAnimInstance());
	if (isSniping || isGatling || isMine || bIsBombingActive)
	{
		AlastProjectCharacter* playerCharacter = Cast<AlastProjectCharacter>(ControlledCharacter);
		if (playerCharacter)
		{
			playerCharacter->StopGatlingGun();
			playerCharacter->DefaultGunComp->SetVisibility(true);
			playerCharacter->sniperGunComp->SetVisibility(false);
			playerCharacter->GatlingGunComp->SetVisibility(false);

		}

		if (BombingRangeActor)
		{
			BombingRangeActor->Destroy();
			BombingRangeActor = nullptr;
		}
		anim->Montage_Stop(0.2f);
		CrosshairWidget->SetVisibility(ESlateVisibility::Hidden); // Crosshair �����
		isSniping = false;
		isGatling = false;
		isMine = false;
		bIsBombingActive = false;
	}
	//anim->PlayerRollAnim("Roll");*/

	FVector RollDirection = ControlledCharacter->GetActorForwardVector();

	ControlledCharacter->LaunchCharacter(RollDirection * RollDistance / RollDuration, true, true);

	StopMovement();
	StartSkillCooldown(RollCooldownTime, SkillCooldownWidget->RollSkillProgressBar, SkillCooldownWidget->RollTextBox, RollElapsedTime, RollCooldownHandle, bCanUseRoll);
}



void AlastProjectPlayerController::Fire()
{
	if (isDie)
		return;


	if (isGatling || bIsBombingActive)
		return;

	if (!isSniping)
	{
		AlastProjectCharacter* ControlledCharacter = Cast<AlastProjectCharacter>(GetPawn());
		if (ControlledCharacter)
		{
			ControlledCharacter->Fire();
		}
	}
	else
	{
		SpawnSniperImpact();
	}
	
}

void AlastProjectPlayerController::Gatling()
{
	if (isDie)
		return;


	if (isSniping || isMine || bIsBombingActive )
		return;

	AlastProjectCharacter* ControlledCharacter = Cast<AlastProjectCharacter>(GetPawn());
	if (ControlledCharacter)
	{
		if (!ControlledCharacter->bCanUseGatling)
			return;

		isGatling = true;
		UPlayerAnim* anim = Cast<UPlayerAnim>(ControlledCharacter->GetMesh()->GetAnimInstance());
		ControlledCharacter->StartGatlingGun();
		anim->PlayerGatlingAnim();
		if (GatlingSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, GatlingSound, GetPawn()->GetActorLocation());
		}
	}
}

void AlastProjectPlayerController::Sniper()
{
	if (isDie)
		return;


	if (isMine || isGatling || bIsBombingActive || !bCanUseSniper)
		return;

	StopMovement();
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn) return;

	AlastProjectCharacter* ControlledCharacter = Cast<AlastProjectCharacter>(GetPawn());
	if (!ControlledCharacter) return;

	UPlayerAnim* anim = Cast<UPlayerAnim>(ControlledCharacter->GetMesh()->GetAnimInstance());

	isSniping = !isSniping;
	if (isSniping)
	{
		ControlledCharacter->sniperGunComp->SetVisibility(true);
		ControlledCharacter->DefaultGunComp->SetVisibility(false);
		anim->PlayerSniperAnim("Sniper");
		if (CrosshairWidget)
		{
			CrosshairWidget->SetVisibility(ESlateVisibility::Visible); // Crosshair �ٽ� ǥ��
		}
	}
	else
	{
		ControlledCharacter->sniperGunComp->SetVisibility(false);
		ControlledCharacter->DefaultGunComp->SetVisibility(true);
		anim->PlayerSniperAnim("Stand");
		if (CrosshairWidget)
		{
			CrosshairWidget->SetVisibility(ESlateVisibility::Hidden); // Crosshair �����
		}
	}

	// ��Ÿ�� ����
	
	StartSkillCooldown(SniperCooldownTime, SkillCooldownWidget->SniperSkillProgressBar, SkillCooldownWidget->SniperTextBox, SniperElapsedTime, SniperCooldownHandle, bCanUseSniper);
}



void AlastProjectPlayerController::UpdateCrosshairPosition()
{
	if (CrosshairWidget)
	{
		FVector2D MousePosition;
		if (GetMousePosition(MousePosition.X, MousePosition.Y))
		{

			// DPI ���� �� ��������
			float DPIScale = GetInputScale();

			FVector2D WidgetPosition = MousePosition / DPIScale;

			FVector2D Offset(-170.0f, -160.0f);
			FVector2D AdjustedWidgetPosition = WidgetPosition + Offset;
			CrosshairWidget->SetPositionInViewport(AdjustedWidgetPosition, true);
		}
	}
}

float AlastProjectPlayerController::GetInputScale() const
{
	if (GEngine && GEngine->GameViewport)
	{
		return GEngine->GameViewport->GetDPIScale();
	}
	return 1.0f;
}

void AlastProjectPlayerController::SpawnSniperImpact()
{
	if (!isSniping) return; // Sniping ���°� �ƴ� �� �������� ����

	FVector2D ScreenLocation;
	if (GetMousePosition(ScreenLocation.X, ScreenLocation.Y))
	{
		FVector WorldLocation, WorldDirection;

		// ȭ�� ��ǥ�� ���� ��ǥ�� ��ȯ
		if (DeprojectScreenPositionToWorld(ScreenLocation.X, ScreenLocation.Y, WorldLocation, WorldDirection))
		{
			// ���� Ʈ���̽��� �����Ͽ� Ÿ�� ��ġ�� ���
			FVector TraceStart = WorldLocation;
			FVector TraceEnd = TraceStart + (WorldDirection * 10000.0f); // 10,000 ���� �Ÿ�

			FHitResult HitResult;
			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(GetPawn()); // �ڽ��� ĳ���ʹ� ����

			bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams);

			if (bHit)
			{
				// ��Ʈ�� ��ġ�� Ÿ�� ȿ�� ����
				if (SniperImpactEffect)
				{
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SniperImpactEffect, HitResult.Location);
				}
				// �������� ���� ���
				if (SniperSound)
				{
					UGameplayStatics::PlaySoundAtLocation(this, SniperSound, GetPawn()->GetActorLocation());
				}

				// ������: ���� �� ��Ʈ ���� ǥ��
				DrawDebugLine(GetWorld(), TraceStart, HitResult.Location, FColor::Green, false, 1.0f);
				DrawDebugSphere(GetWorld(), HitResult.Location, 10.0f, 12, FColor::Red, false, 1.0f);

				auto Boss = HitResult.GetActor()->GetDefaultSubobjectByName(TEXT("FSM"));
				if (Boss)
				{
					UBossFSM* bossFSM = Cast<UBossFSM>(Boss);
					bossFSM->OnDamageProcess(5.0f);
				}
			}
			else
			{
				// ������: ��Ʈ���� �ʾ��� ����� ���� ǥ��
				DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, 1.0f);
			}
		}
	}
	
}



void AlastProjectPlayerController::SpawnMine()
{
	if (isDie)
		return;

	if (!bCanUseMine)
		return;

	if (!Mine || isGatling || isSniping || bIsBombingActive) return;

	isMine = true;
	StopMovement();
	// �÷��̾� ĳ���� ��������
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn) return;
	// ���� ��ġ ����: �÷��̾� ��ġ + ���� ���� * ���� �Ÿ�
	this->SpawnLocation = ControlledPawn->GetActorLocation() + ControlledPawn->GetActorForwardVector() * SpawnDistance;
	FRotator SpawnRotation = FRotator::ZeroRotator; // ȸ���� �ʿ信 ���� ���� ����

	AlastProjectCharacter* ControlledCharacter = Cast<AlastProjectCharacter>(GetPawn());
	if (!ControlledCharacter) return;

	UPlayerAnim* anim = Cast<UPlayerAnim>(ControlledCharacter->GetMesh()->GetAnimInstance());
	if (anim)
	{
		anim->PlayerMineAnim(); // ��Ÿ�� ��ȯ �Լ�
		UAnimMontage* CurrentMontage = anim->GetCurrentActiveMontage();
		if (CurrentMontage)
		{
			// �ִϸ��̼� ���� �̺�Ʈ ���ε�
			FOnMontageEnded EndDelegate;
			EndDelegate.BindUObject(this, &AlastProjectPlayerController::OnMineAnimationFinished);
			anim->Montage_SetEndDelegate(EndDelegate, CurrentMontage);
		}
	}


	// ���忡 ���� ����
	UWorld* World = GetWorld();
	if (World)
	{
		World->SpawnActor<AMine>(Mine, SpawnLocation, SpawnRotation);
	}

	StartSkillCooldown(MineCooldownTime, SkillCooldownWidget->MineSkillProgressBar, SkillCooldownWidget->MineTextBox, MineElapsedTime, MineCooldownHandle, bCanUseMine);
}


void AlastProjectPlayerController::OnMineAnimationFinished(UAnimMontage* Montage, bool bInterrupted)
{
	isMine = false;
}

void AlastProjectPlayerController::StartBombing()
{
	if (isDie)
		return;


	if (bIsBombingActive || !bCanUseBombing) return;

	if (isMine || isSniping || isGatling) return;

	bIsBombingActive = true;

	FVector WorldLocation, WorldDirection;
	if (DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
	{
		FVector TraceStart = WorldLocation;
		FVector TraceEnd = TraceStart + WorldDirection * 10000.0f;

		FHitResult HitResult;
		if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility))
		{
			BombingTargetLocation = HitResult.Location;

			// ��Į ���� ����
			BombingRangeActor = GetWorld()->SpawnActor<ADecalActor>(HitResult.Location, FRotator::ZeroRotator);
			if (BombingRangeActor)
			{
				BombingRangeActor->SetDecalMaterial(BombingRangeMaterial); // ��Ƽ���� ����
				BombingRangeActor->GetDecal()->DecalSize = FVector(500.0f, 500.0f, 500.0f); // ���� ũ�� ����
			}
		}
	}
}

void AlastProjectPlayerController::ConfirmBombing()
{
	if (!bIsBombingActive || !bCanUseBombing) return;

	bIsBombingActive = false;

	// ���� ǥ�� ����
	if (BombingRangeActor)
	{
		BombingRangeActor->Destroy();
		BombingRangeActor = nullptr;
	}

	// ����Ʈ �߻�
	if (BombingEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BombingEffect, BombingTargetLocation, FRotator::ZeroRotator, true);
	}
	// ���� ��ų ���� ���
	if (BombingSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, BombingSound, BombingTargetLocation);
	}


	float BombingRadius = 500.0f;
	TArray<FOverlapResult> OverlapResults;
	FCollisionShape CollisionShape = FCollisionShape::MakeSphere(BombingRadius);

	bool bHit = GetWorld()->OverlapMultiByChannel(
		OverlapResults,
		BombingTargetLocation,
		FQuat::Identity,
		ECC_Pawn,
		CollisionShape
	);

	for (const FOverlapResult& Result : OverlapResults)
	{
		ABoss* HitBoss = Cast<ABoss>(Result.GetActor());
		if (HitBoss)
		{
			UBossFSM* bossFSM = Cast<UBossFSM>(HitBoss->GetComponentByClass(UBossFSM::StaticClass()));
			if (bossFSM)
			{
				float BombDamage = 30.0f;
				bossFSM->OnDamageProcess(BombDamage);
			}
		}
	}


	StartSkillCooldown(BombingCooldownTime, SkillCooldownWidget->BombingSkillProgressBar, SkillCooldownWidget->BombingTextBox, BombingElapsedTime, BombingCooldownHandle, bCanUseBombing);
}



void AlastProjectPlayerController::StartSkillCooldown(
	float CooldownTime,
	UProgressBar* ProgressBar,
	UTextBlock* TextBox,
	float& ElapsedTime,
	FTimerHandle& TimerHandle,
	bool& bCanUseSkill) // ��ų ��� ���� ���� ���� �߰�
{
	if (GetWorldTimerManager().IsTimerActive(TimerHandle))
	{
		GetWorldTimerManager().ClearTimer(TimerHandle);
	}

	ElapsedTime = 0.0f; // ��� �ð� �ʱ�ȭ
	bCanUseSkill = false; // ��ų ��� �Ұ� ���� ����
	TimerInterval = 0.1f; // Ÿ�̸� �ֱ� ����

	GetWorldTimerManager().SetTimer(TimerHandle,
		[this, &ElapsedTime, ProgressBar, TextBox, CooldownTime, &bCanUseSkill, &TimerHandle]() mutable
		{
			// ��� �ð� ������Ʈ
			ElapsedTime += TimerInterval;

			if (SkillCooldownWidget)
			{
				SkillCooldownWidget->UpdateSkillCooldown(ProgressBar, TextBox, CooldownTime, ElapsedTime);
			}

			// ��Ÿ�� ���� ó��
			if (ElapsedTime >= CooldownTime)
			{
				GetWorldTimerManager().ClearTimer(TimerHandle);
				bCanUseSkill = true; // ��ų ��� ���� ���·� ����
				ElapsedTime = 0.0f;  // ��� �ð� �ʱ�ȭ
			}
		},
		0.1f, true);
}