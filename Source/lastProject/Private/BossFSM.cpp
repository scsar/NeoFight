// Fill out your copyright notice in the Description page of Project Settings.


#include "BossFSM.h"
#include "Boss.h"
#include "lastProjectCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "Components/DecalComponent.h"
#include "Engine/OverlapResult.h"
#include "BossAnim.h"

// Sets default values for this component's properties
UBossFSM::UBossFSM()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	MaxHP = 200.0f;
	CurrentHP = MaxHP;
}


// Called when the game starts
void UBossFSM::BeginPlay()
{
	Super::BeginPlay();


	auto actor = UGameplayStatics::GetActorOfClass(GetWorld(), AlastProjectCharacter::StaticClass());
	target = Cast<AlastProjectCharacter>(actor);
	me = Cast<ABoss>(GetOwner());

	anim = Cast<UBossAnim>(me->GetMesh()->GetAnimInstance());

	ai = Cast<AAIController>(me->GetController());

	if (AActor* OwnerActor = GetOwner()) // FSM의 소유자 Actor 가져오기
	{
		ABoss* Character = Cast<ABoss>(OwnerActor);
		if (Character && Character->GetCharacterMovement())
		{
			Character->GetCharacterMovement()->MaxWalkSpeed = 300.0f; // 이동 속도 설정
		}
	}
	
	if (BossHUDWidgetClass)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			BossHUDWidget = CreateWidget<UPlayerHUDWidget>(World, BossHUDWidgetClass);
			if (BossHUDWidget)
			{
				BossHUDWidget->AddToViewport();
				BossHUDWidget->UpdateHPBar(CurrentHP, MaxHP);
				UE_LOG(LogTemp, Log, TEXT("BossHUDWidget created successfully."));
			}
		}
	}
}


// Called every frame
void UBossFSM::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	switch (mState)
	{
	case EBossState::Idle:
		IdleState();
		break;
	case EBossState::Move:
		MoveState();
		break;
	case EBossState::Attack:
		AttackState();
		break;
	case EBossState::Damage:
		DamageState();
		break;
	case EBossState::Die:
		DieState();
		break;
	}
}

void UBossFSM::IdleState()
{
	if (CurrentHP >= 100.f)
	{
		mState = EBossState::Move;
	}
	else
	{
		currentTime += GetWorld()->DeltaTimeSeconds;
		if (currentTime > idleDelayTime)
		{
			mState = EBossState::Move;
			currentTime = 0;

			anim->animState = mState;
		}
	}
}

void UBossFSM::MoveState()
{
	FVector destination = target->GetActorLocation();
	FVector dir = destination - me->GetActorLocation();
	//me->AddMovementInput(dir.GetSafeNormal());
	//ai->MoveToLocation(destination);

	auto ns = UNavigationSystemV1::GetNavigationSystem(GetWorld());

	FPathFindingQuery query;
	FAIMoveRequest req;
	req.SetAcceptanceRadius(3);
	req.SetGoalLocation(destination);

	ai->BuildPathfindingQuery(req, query);

	FPathFindingResult r = ns->FindPathSync(query);
	if (r.Result == ENavigationQueryResult::Success)
	{
		ai->MoveToLocation(destination);
	}

	if (dir.Size() < attackRange)
	{
		ai->StopMovement();

		mState = EBossState::Attack;

		anim->animState = mState;
	}
}

void UBossFSM::AttackState()
{
	currentTime += GetWorld()->DeltaTimeSeconds;
	if (currentTime > attackDelayTime)
	{
		if (FMath::RandRange(0, 100) < 30)  // 30% 확률로 발동
		{
			ActivateAreaSkill();
			return;  // 스킬 사용 시 바로 리턴
		}
		else
		{
			ActivateLaserSkill();
		}
		currentTime = 0;
	}

	float distance = FVector::Distance(target->GetActorLocation(), me->GetActorLocation());
	if (distance > attackRange)
	{
		mState = EBossState::Move;
		anim->animState = mState;

	}
}


void UBossFSM::DamageState()
{
	if (CurrentHP >= 150)
	{
		currentTime += GetWorld()->DeltaTimeSeconds;
		if (currentTime > damageDelayTime)
		{
			mState = EBossState::Idle;
			currentTime = 0;
			anim->animState = mState;
		
		}
	}
	else
	{
		mState = EBossState::Idle;
		currentTime = 0;
	}
}

void UBossFSM::DieState()
{
	/*if (anim->bdieDone == false)
	{
		return;
	}*/
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PlayerController && ClearWidgetClass)
	{
		UUserWidget* BossClearWidget = CreateWidget<UUserWidget>(PlayerController, ClearWidgetClass);
		if (BossClearWidget)
		{
			BossClearWidget->AddToViewport();
		}
	}
	FVector P0 = me->GetActorLocation();
	FVector vt = FVector::DownVector * dieSpeed * GetWorld()->DeltaTimeSeconds;
	FVector P = P0 + vt;
	me->SetActorLocation(P);

	if (P.Z < -200.0f)
	{
		me->Destroy();
	}
}

void UBossFSM::OnDamageProcess(float DamageAmount)
{
	CurrentHP = FMath::Clamp(CurrentHP - DamageAmount, 0.0f, MaxHP);

	if (CurrentHP > 0)
	{
			UPlayerHUDWidget* HUDWidget = Cast<UPlayerHUDWidget>(BossHUDWidget);
			if (HUDWidget)
			{
				HUDWidget->UpdateHPBar(CurrentHP, MaxHP);
			}

		mState = EBossState::Damage;

		int32 index = FMath::RandRange(0, 2);
		FString sectionName = FString::Printf(TEXT("Damage%d"), index);
		anim->BossDamageAnim();
	}
	else
	{
		mState = EBossState::Die;
		me->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		anim->BossDamageAnim();
	}
	anim->animState = mState;

	ai->StopMovement();
}

void UBossFSM::ActivateLaserSkill()
{
	if (!target || !me) return;

	// 1. 플레이어의 위치 저장
	LaserTargetLocation = target->GetActorLocation();

	// 2. 보스 위치에서 플레이어까지 레이저를 발사 (시각적 디버그)
	DrawDebugLine(
		GetWorld(),
		me->GetActorLocation(),    // 레이저 시작 위치 (보스 위치)
		LaserTargetLocation,       // 레이저 끝 위치 (플레이어 위치)
		FColor::Red,               // 레이저 색상
		false,
		1.0f,                      // 레이저 표시 시간
		0,
		5.0f                       // 레이저 두께
	);

	// 3. LineTraceSingleByChannel 사용하여 충돌 감지
	FHitResult HitResult;
	FVector StartLocation = me->GetActorLocation();
	FVector EndLocation = LaserTargetLocation;

	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(me); // 보스는 충돌 제외

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		StartLocation,
		EndLocation,
		ECC_Pawn, // 플레이어 같은 Pawn 채널
		CollisionParams
	);

	// 4. 플레이어에 데미지 적용
	if (bHit && HitResult.GetActor())
	{
		AlastProjectCharacter* HitPlayer = Cast<AlastProjectCharacter>(HitResult.GetActor());
		if (HitPlayer)
		{
			float LaserDamage = 10.0f; // 레이저 데미지
			HitPlayer->TakeDamage(LaserDamage);

			UE_LOG(LogTemp, Warning, TEXT("Player hit by laser! Damage: %f"), LaserDamage);
		}
	}
}

void UBossFSM::ActivateAreaSkill()
{
	if (!me || !target) return;

	// 플레이어의 현재 위치를 저장 (스킬 범위의 중심)
	SkillTargetLocation = target->GetActorLocation();

	// 데칼(범위 표시) 생성
	ADecalActor* SkillRangeIndicator = GetWorld()->SpawnActor<ADecalActor>(SkillTargetLocation, FRotator::ZeroRotator);
	if (SkillRangeIndicator)
	{
		SkillRangeIndicator->SetDecalMaterial(SkillRangeMaterial); // 머티리얼 설정
		SkillRangeIndicator->GetDecal()->DecalSize = FVector(500.0f, 500.0f, 500.0f); // 범위 크기 설정
		SkillRangeIndicator->SetLifeSpan(SkillDelayTime);  // 일정 시간 후 데칼 제거
	}

	// 일정 시간 후 데미지 적용 함수 호출
	GetWorld()->GetTimerManager().SetTimer(SkillTimerHandle, this, &UBossFSM::ApplyAreaSkillDamage, SkillDelayTime, false);
}

void UBossFSM::ApplyAreaSkillDamage()
{
	// Sphere Overlap을 통해 범위 내 액터 감지
	TArray<FOverlapResult> OverlapResults;
	FCollisionShape CollisionShape = FCollisionShape::MakeSphere(SkillRadius);

	bool bHit = GetWorld()->OverlapMultiByChannel(
		OverlapResults,
		SkillTargetLocation,
		FQuat::Identity,
		ECC_Pawn,
		CollisionShape
	);

	for (const FOverlapResult& Result : OverlapResults)
	{
		AlastProjectCharacter* Player = Cast<AlastProjectCharacter>(Result.GetActor());
		if (Player)
		{
			// 데미지 적용
			Player->TakeDamage(SkillDamage);
			return;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Area Skill Damage Applied!"));
}