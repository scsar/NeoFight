// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "BossFSM.h"
#include "BossAnim.generated.h"

/**
 * 
 */
UCLASS()
class LASTPROJECT_API UBossAnim : public UAnimInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = FSM)
	EBossState animState;

	UFUNCTION(BlueprintImplementableEvent, Category = FSMEvent)
	void BossDamageAnim();

	UFUNCTION(BlueprintImplementableEvent, Category = FSMEvent)
	void BossDieAnim();

};
