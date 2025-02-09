// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerHUDWidget.h"
#include "Blueprint/UserWidget.h"
#include "Engine/DecalActor.h"
#include "BossFSM.generated.h"

UENUM(BlueprintType)
enum class EBossState : uint8
{
	Idle,
	Move,
	Attack,
	Damage,
	Die
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LASTPROJECT_API UBossFSM : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBossFSM();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = FSM)
	EBossState mState = EBossState::Idle;

	void IdleState();
	void MoveState();
	void AttackState();
	void DamageState();
	void DieState();

	void OnDamageProcess(float DamageAmount);
	void ActivateLaserSkill();

	// 공격 범위 관련 변수
	FVector SkillTargetLocation;        // 범위 공격의 중심 위치
	float SkillRadius = 500.0f;         // 범위 반경
	float SkillDelayTime = 3.0f;        // 스킬 발동까지의 딜레이 시간
	float SkillDamage = 20.0f;          // 범위 내 플레이어에게 주는 데미지

	FTimerHandle SkillTimerHandle;      // 타이머 핸들

	// 공격 스킬 함수
	void ActivateAreaSkill();
	void ApplyAreaSkillDamage();

	FVector LaserTargetLocation;

	UPROPERTY(EditDefaultsOnly, Category = FSM)
	float idleDelayTime = 2.0f;

	float currentTime = 0;

	UPROPERTY(VisibleAnywhere, Category = FSM)
	class AlastProjectCharacter* target;

	UPROPERTY()
	class ABoss* me;

	UPROPERTY()
	class AAIController* ai;

	UPROPERTY(EditAnywhere, Category = FSM)
	float attackRange = 1500.0f;

	UPROPERTY(EditAnywhere, Category = FSM)
	float attackDelayTime = 2.0f;

	/** HP 변수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = FSM)
	float CurrentHP;

	/** 최대 HP */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = FSM)
	float MaxHP;

	UPROPERTY(EditAnywhere, Category = FSM)
	float damageDelayTime = 1.0f;

	UPROPERTY(EditAnywhere, Category = FSM)
	float dieSpeed = 50.0f;

	UPROPERTY()
	UPlayerHUDWidget* BossHUDWidget;

	UPROPERTY(EditDefaultsOnly, Category = UI)
	TSubclassOf<UUserWidget> BossHUDWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = UI)
	TSubclassOf<UUserWidget> ClearWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = Skill)
	UMaterialInterface* SkillRangeMaterial;

	ADecalActor* SkillRangeActor; // 스킬 범위 표시용 데칼 액터

	UPROPERTY()
	class UBossAnim* anim;
};
