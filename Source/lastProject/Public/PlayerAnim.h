// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "lastProjectCharacter.h"
#include "PlayerAnim.generated.h"

/**
 * 
 */
UCLASS()
class LASTPROJECT_API UPlayerAnim : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = PlayerAnim)
	float speed = 0;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = PlayerAnim)
	bool ShouldMove;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = PlayerAnim)
	FVector velocity;

	UPROPERTY(BlueprintReadWrite, Category = "Movement")
	FVector CurrentAcceleration;

	UPROPERTY(BlueprintReadWrite, Category = "Movement")
	float MaxDistance = 1000.0f; // 최대 거리

	// 캐릭터와 Movement Component 참조
	UPROPERTY(BlueprintReadOnly, Category = "PlayerAnim")
	ACharacter* Character;

	UPROPERTY(BlueprintReadOnly, Category = "PlayerAnim")
	UCharacterMovementComponent* MovementComponent;

	UFUNCTION(BlueprintImplementableEvent, Category = FSMEvent)
	void PlayerRollAnim(FName sectioName);

	UFUNCTION(BlueprintImplementableEvent, Category = FSMEvent)
	void PlayerSniperAnim(FName sectionName);

	UFUNCTION(BlueprintImplementableEvent, Category = FSMEvent)
	void PlayerGatlingAnim();

	UFUNCTION(BlueprintImplementableEvent, Category = FSMEvent)
	void PlayerMineAnim();
	
	UFUNCTION(BlueprintImplementableEvent, Category = FSMEvent)
	void PlayerDieAnim();



	FVector CachedDestination;
	FVector LastValidAcceleration = FVector::ZeroVector;
	AController* BaseController;


	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	virtual void NativeInitializeAnimation() override;
};
