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

	// 초기값 설정
	bCanUseSniper = true;
	bCanUseMine = true;
	bCanUseBombing = true;
	bCanUseRoll = true;

	SniperCooldownTime = 2.0f;   // 2초 쿨타임
	MineCooldownTime = 8.0f;    // 8초 쿨타임
	BombingCooldownTime = 12.0f; // 12초 쿨타임
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
			BGMComponent->SetVolumeMultiplier(0.5f); // 볼륨 조절 (0.0 ~ 1.0)
		}
	}

	// Crosshair Widget 생성 및 화면에 추가
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

	// Crosshair 위치 업데이트
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
		CrosshairWidget->SetVisibility(ESlateVisibility::Hidden); // Crosshair 숨기기
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
			CrosshairWidget->SetVisibility(ESlateVisibility::Visible); // Crosshair 다시 표시
		}
	}
	else
	{
		ControlledCharacter->sniperGunComp->SetVisibility(false);
		ControlledCharacter->DefaultGunComp->SetVisibility(true);
		anim->PlayerSniperAnim("Stand");
		if (CrosshairWidget)
		{
			CrosshairWidget->SetVisibility(ESlateVisibility::Hidden); // Crosshair 숨기기
		}
	}

	// 쿨타임 시작
	
	StartSkillCooldown(SniperCooldownTime, SkillCooldownWidget->SniperSkillProgressBar, SkillCooldownWidget->SniperTextBox, SniperElapsedTime, SniperCooldownHandle, bCanUseSniper);
}



void AlastProjectPlayerController::UpdateCrosshairPosition()
{
	if (CrosshairWidget)
	{
		FVector2D MousePosition;
		if (GetMousePosition(MousePosition.X, MousePosition.Y))
		{

			// DPI 보정 값 가져오기
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
	if (!isSniping) return; // Sniping 상태가 아닐 때 실행하지 않음

	FVector2D ScreenLocation;
	if (GetMousePosition(ScreenLocation.X, ScreenLocation.Y))
	{
		FVector WorldLocation, WorldDirection;

		// 화면 좌표를 월드 좌표로 변환
		if (DeprojectScreenPositionToWorld(ScreenLocation.X, ScreenLocation.Y, WorldLocation, WorldDirection))
		{
			// 라인 트레이스를 실행하여 타격 위치를 계산
			FVector TraceStart = WorldLocation;
			FVector TraceEnd = TraceStart + (WorldDirection * 10000.0f); // 10,000 단위 거리

			FHitResult HitResult;
			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(GetPawn()); // 자신의 캐릭터는 무시

			bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams);

			if (bHit)
			{
				// 히트한 위치에 타격 효과 생성
				if (SniperImpactEffect)
				{
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SniperImpactEffect, HitResult.Location);
				}
				// 스나이퍼 사운드 재생
				if (SniperSound)
				{
					UGameplayStatics::PlaySoundAtLocation(this, SniperSound, GetPawn()->GetActorLocation());
				}

				// 디버깅용: 라인 및 히트 지점 표시
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
				// 디버깅용: 히트하지 않았을 경우의 라인 표시
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
	// 플레이어 캐릭터 가져오기
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn) return;
	// 스폰 위치 설정: 플레이어 위치 + 전방 방향 * 스폰 거리
	this->SpawnLocation = ControlledPawn->GetActorLocation() + ControlledPawn->GetActorForwardVector() * SpawnDistance;
	FRotator SpawnRotation = FRotator::ZeroRotator; // 회전은 필요에 따라 설정 가능

	AlastProjectCharacter* ControlledCharacter = Cast<AlastProjectCharacter>(GetPawn());
	if (!ControlledCharacter) return;

	UPlayerAnim* anim = Cast<UPlayerAnim>(ControlledCharacter->GetMesh()->GetAnimInstance());
	if (anim)
	{
		anim->PlayerMineAnim(); // 몽타주 반환 함수
		UAnimMontage* CurrentMontage = anim->GetCurrentActiveMontage();
		if (CurrentMontage)
		{
			// 애니메이션 종료 이벤트 바인딩
			FOnMontageEnded EndDelegate;
			EndDelegate.BindUObject(this, &AlastProjectPlayerController::OnMineAnimationFinished);
			anim->Montage_SetEndDelegate(EndDelegate, CurrentMontage);
		}
	}


	// 월드에 마인 스폰
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

			// 데칼 액터 스폰
			BombingRangeActor = GetWorld()->SpawnActor<ADecalActor>(HitResult.Location, FRotator::ZeroRotator);
			if (BombingRangeActor)
			{
				BombingRangeActor->SetDecalMaterial(BombingRangeMaterial); // 머티리얼 설정
				BombingRangeActor->GetDecal()->DecalSize = FVector(500.0f, 500.0f, 500.0f); // 범위 크기 설정
			}
		}
	}
}

void AlastProjectPlayerController::ConfirmBombing()
{
	if (!bIsBombingActive || !bCanUseBombing) return;

	bIsBombingActive = false;

	// 범위 표시 제거
	if (BombingRangeActor)
	{
		BombingRangeActor->Destroy();
		BombingRangeActor = nullptr;
	}

	// 이펙트 발생
	if (BombingEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BombingEffect, BombingTargetLocation, FRotator::ZeroRotator, true);
	}
	// 폭격 스킬 사운드 재생
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
	bool& bCanUseSkill) // 스킬 사용 가능 상태 변수 추가
{
	if (GetWorldTimerManager().IsTimerActive(TimerHandle))
	{
		GetWorldTimerManager().ClearTimer(TimerHandle);
	}

	ElapsedTime = 0.0f; // 경과 시간 초기화
	bCanUseSkill = false; // 스킬 사용 불가 상태 설정
	TimerInterval = 0.1f; // 타이머 주기 설정

	GetWorldTimerManager().SetTimer(TimerHandle,
		[this, &ElapsedTime, ProgressBar, TextBox, CooldownTime, &bCanUseSkill, &TimerHandle]() mutable
		{
			// 경과 시간 업데이트
			ElapsedTime += TimerInterval;

			if (SkillCooldownWidget)
			{
				SkillCooldownWidget->UpdateSkillCooldown(ProgressBar, TextBox, CooldownTime, ElapsedTime);
			}

			// 쿨타임 종료 처리
			if (ElapsedTime >= CooldownTime)
			{
				GetWorldTimerManager().ClearTimer(TimerHandle);
				bCanUseSkill = true; // 스킬 사용 가능 상태로 변경
				ElapsedTime = 0.0f;  // 경과 시간 초기화
			}
		},
		0.1f, true);
}