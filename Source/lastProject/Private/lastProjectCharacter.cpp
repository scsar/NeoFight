// Copyright Epic Games, Inc. All Rights Reserved.

#include "lastProjectCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Materials/Material.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Bullet.h"
#include "PlayerAnim.h"
#include "lastProjectPlayerController.h"



AlastProjectCharacter::AlastProjectCharacter()
{
	// Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Rotate character to moving direction
	GetCharacterMovement()->RotationRate = FRotator(0.f, 800.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// Create a camera boom...
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true); // Don't want arm to rotate when character does
	CameraBoom->TargetArmLength = 800.f;
	CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level

	// Create a camera...
	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	DefaultGunComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DefaultGun"));
	//DefaultGunComp->SetupAttachment(GetMesh());
	DefaultGunComp->SetupAttachment(GetMesh(), TEXT("LeftWeaphonSocket"));

	sniperGunComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SniperGun"));
	sniperGunComp->SetupAttachment(GetMesh(), TEXT("LeftWeaphonSocket"));

	GatlingGunComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GatlingGun"));
	GatlingGunComp->SetupAttachment(GetMesh());

	GatlingGunSocketNames = {
		FName(TEXT("FirePosition_1")),
		FName(TEXT("FirePosition_2")),
		FName(TEXT("FirePosition_3")),
		FName(TEXT("FirePosition_4")),
		FName(TEXT("FirePosition_5")),
		FName(TEXT("FirePosition_6")),
		FName(TEXT("FirePosition_7"))
	};

	bCanUseGatling = true;
	GatlingCooldownTime = 8.0f; // 쿨타임 8초
	isDie = false;

	MaxHP = 100.0f;
	CurrentHP = MaxHP;

	// Activate ticking in order to update the cursor every frame.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

}

void AlastProjectCharacter::BeginPlay()
{
	Super::BeginPlay();

	sniperGunComp->SetVisibility(false);
	GatlingGunComp->SetVisibility(false);
}

void AlastProjectCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
}

void AlastProjectCharacter::Fire()
{
	if (!DefaultGunComp) return;

	// 총알 클래스 확인
	if (bulletFactory)
	{
		// 월드에 총알 스폰
		UWorld* World = GetWorld();
		if (World)
		{
			FTransform firePosition = DefaultGunComp->GetSocketTransform(TEXT("FirePosition"));
			World->SpawnActor<ABullet>(bulletFactory, firePosition);
		}
	}
}

void AlastProjectCharacter::FireGatlingGun()
{
	if (!GatlingGunComp || !bulletFactory) return;

	// 랜덤 소켓 이름 선택
	int32 RandomIndex = FMath::RandRange(0, GatlingGunSocketNames.Num() - 1);
	FName SelectedSocket = GatlingGunSocketNames[RandomIndex];

	// 월드에 총알 스폰
	UWorld* World = GetWorld();
	if (World)
	{
		FTransform FireTransform = GatlingGunComp->GetSocketTransform(SelectedSocket);
		World->SpawnActor<ABullet>(bulletFactory, FireTransform);
	}
}

void AlastProjectCharacter::StartGatlingGun()
{
	if (!bCanUseGatling)
		return;


	if (FireTimerHandle.IsValid()) 
		return; // 이미 발사 중인 경우 무시

	DefaultGunComp->SetVisibility(false);
	sniperGunComp->SetVisibility(false);
	GatlingGunComp->SetVisibility(true);

	UCharacterMovementComponent* MovementComp = GetCharacterMovement();
	if (MovementComp)
	{
		float OriginalSpeed = MovementComp->MaxWalkSpeed; 
		MovementComp->MaxWalkSpeed = 300.0f; 
	}

	// 일정 간격으로 총알 발사
	GetWorldTimerManager().SetTimer(FireTimerHandle, this, &AlastProjectCharacter::FireGatlingGun, FireInterval, true);

	// FireDuration 이후 Gatling 중지
	FTimerHandle StopFireTimerHandle;
	GetWorldTimerManager().SetTimer(StopFireTimerHandle, this, &AlastProjectCharacter::StopGatlingGun, FireDuration, false);
	
	// PlayerController에 쿨타임 UI 업데이트 요청
	if (AController* TempController = GetController())
	{
		if (AlastProjectPlayerController* PlayerController = Cast<AlastProjectPlayerController>(Controller))
		{
			PlayerController->isGatling = false;
			PlayerController->StartSkillCooldown(
				GatlingCooldownTime,
				PlayerController->SkillCooldownWidget->GatlingSkillProgressBar,
				PlayerController->SkillCooldownWidget->GatlingTextBox, GatlingElapsedTime,
				GatlingCooldownHandle, bCanUseGatling
			);
		}
	}
}

void AlastProjectCharacter::StopGatlingGun()
{
	// 발사 타이머 중지
	GetWorldTimerManager().ClearTimer(FireTimerHandle);

	// 무기 가시성 설정
	GatlingGunComp->SetVisibility(false);
	DefaultGunComp->SetVisibility(true);


	// 쿨타임 시작
	//bCanUseGatling = false;
	UPlayerAnim* anim = Cast<UPlayerAnim>(GetMesh()->GetAnimInstance());
	anim->Montage_Stop(0.2f);


	UCharacterMovementComponent* MovementComp = GetCharacterMovement();
	if (MovementComp)
	{
		float OriginalSpeed = MovementComp->MaxWalkSpeed;
		MovementComp->MaxWalkSpeed = 600.0f;
	}
}


void AlastProjectCharacter::TakeDamage(float DamageAmount)
{
	if (isDie) return;

	// 체력 감소
	CurrentHP = FMath::Clamp(CurrentHP - DamageAmount, 0.0f, MaxHP);

	AlastProjectPlayerController* PC = Cast<AlastProjectPlayerController>(GetController());
	if (PC)
	{
		UPlayerHUDWidget* HUDWidget = Cast<UPlayerHUDWidget>(PC->playerHUDWidget);
		if (HUDWidget)
		{
			HUDWidget->UpdateHPBar(CurrentHP, MaxHP);
		}
	}

	if (CurrentHP <= 0.0f)
	{
		Die();
	}
}

void AlastProjectCharacter::Die()
{
	UE_LOG(LogTemp, Warning, TEXT("Player has died."));

	AController* TempController = GetController();
	AlastProjectPlayerController* PlayerController = Cast<AlastProjectPlayerController>(TempController);
	if (PlayerController)
	{
		PlayerController->isDie = true;
		UPlayerAnim* anim = Cast<UPlayerAnim>(GetMesh()->GetAnimInstance());
		if (anim)
		{
			anim->PlayerDieAnim();
			isDie = true;
		}
	}

	if (DeathWidgetClass)
	{
		UUserWidget* DeathWidget = CreateWidget<UUserWidget>(GetWorld(), DeathWidgetClass);
		if (DeathWidget)
		{
			DeathWidget->AddToViewport();
		}
	}


	// 사망 처리를 위한 추가 로직 (예: UI 표시, 리스폰 등)
}