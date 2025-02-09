// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Engine/DecalActor.h"
#include "SkillCooldownWidget.h"
#include "PlayerHUDWidget.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "lastProjectPlayerController.generated.h"

/** Forward declaration to improve compiling times */
class UNiagaraSystem;
class UInputMappingContext;
class UInputAction;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS()
class AlastProjectPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AlastProjectPlayerController();

	/** Time Threshold to know if it was a short press */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	float ShortPressThreshold;

	/** FX Class that we will spawn when clicking */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UNiagaraSystem* FXCursor;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;
	
	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* SetDestinationClickAction;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* SetDestinationTouchAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* IA_Roll;

	// 구르기 거리와 속도 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rolling)
	float RollDistance = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rolling)
	float RollDuration = 0.5f;


	bool isSniping = false;
	bool isGatling = false;
	bool isMine = false;
	bool bIsBombingActive;

	FTimerHandle RollTimerHandle;
	FTimerHandle RollCooldownHandle;
	FTimerHandle SniperCooldownHandle;
	FTimerHandle MineCooldownHandle;
	FTimerHandle BombingCooldownHandle;

	float RollElapsedTime = 0.0f;
	float SniperElapsedTime = 0.0f;
	float MineElapsedTime = 0.0f;
	float BombingElapsedTime = 0.0f;

	bool bCanUseSniper;
	bool bCanUseGatling;
	bool bCanUseMine;
	bool bCanUseBombing;
	bool bCanUseRoll;

	UPROPERTY(EditDefaultsOnly, Category = Input)
	UInputAction* IA_Fire;

	UPROPERTY(EditDefaultsOnly, Category = Input)
	UInputAction* IA_Sniper;
	
	UPROPERTY(EditDefaultsOnly, Category = Input)
	UInputAction* IA_Gatling;

	UPROPERTY(EditDefaultsOnly, Category = Input)
	UInputAction* IA_Mine;
	
	UPROPERTY(EditDefaultsOnly, Category = Input)
	UInputAction* IA_Bombing;
	
	UPROPERTY(EditDefaultsOnly, Category = Input)
	UInputAction* IA_BombingActive;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> CrosshairWidgetClass;

	UPROPERTY()
	UUserWidget* CrosshairWidget;

	FVector CachedDestination;

	UPROPERTY(EditDefaultsOnly, Category = Mine)
	TSubclassOf<class AMine> Mine;

	UPROPERTY(EditDefaultsOnly, Category = Mine)
	float SpawnDistance;

	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	UParticleSystem* BombingEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Skill")
	UMaterialInterface* BombingRangeMaterial;

	ADecalActor* BombingRangeActor; // 스킬 범위 표시용 데칼 액터

	FVector BombingTargetLocation;

	// 쿨타임 설정
	UPROPERTY(EditDefaultsOnly, Category = Cooldowns)
	float SniperCooldownTime;

	UPROPERTY(EditDefaultsOnly, Category = Cooldowns)
	float MineCooldownTime;

	UPROPERTY(EditDefaultsOnly, Category = Cooldowns)
	float BombingCooldownTime;
	
	UPROPERTY(EditDefaultsOnly, Category = Cooldowns)
	float RollCooldownTime;

	UPROPERTY()
	USkillCooldownWidget* SkillCooldownWidget;

	UPROPERTY(EditDefaultsOnly, Category = UI)
	TSubclassOf<UUserWidget> SkillCooldownWidgetClass;

	UPROPERTY()
	UPlayerHUDWidget* playerHUDWidget;

	UPROPERTY(EditDefaultsOnly, Category = UI)
	TSubclassOf<UUserWidget> playerHUDWidgetClass;

	UPROPERTY(EditAnywhere, Category = Sound)
	USoundBase* SniperSound;

	UPROPERTY(EditAnywhere, Category = Sound)
	USoundBase* BombingSound;

	UPROPERTY(EditAnywhere, Category = Sound)
	USoundBase* GatlingSound;

	// BGM 변수 선언
	UPROPERTY(EditAnywhere, Category = Sound)
	USoundBase* BackgroundMusic;

	UAudioComponent* BGMComponent;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Effects)
	UParticleSystem* SniperImpactEffect;


protected:
	/** True if the controlled character should navigate to the mouse cursor. */
	uint32 bMoveToMouseCursor : 1;

	virtual void SetupInputComponent() override;
	virtual void Tick(float DeltaTime) override;

	// To add mapping context
	virtual void BeginPlay();

	/** Input handlers for SetDestination action. */
	void OnInputStarted();
	void OnSetDestinationTriggered();
	void OnSetDestinationReleased();
	void OnTouchTriggered();
	void OnTouchReleased();
	void Roll();
	void EndRoll();
	void Fire();

	void Sniper();
	void UpdateCrosshairPosition();
	void SpawnSniperImpact();
	float GetInputScale() const;

	void Gatling();
	void SpawnMine();

	void StartBombing();
	void ConfirmBombing();



	UFUNCTION()
	void OnMineAnimationFinished(UAnimMontage* Montage, bool bInterrupted);
public:
	void StartSkillCooldown(float CooldownTime, UProgressBar* ProgressBar, UTextBlock* TextBox, float& ElapsedTime, FTimerHandle& TimerHandle, bool& bCanUseSkill);
	bool isDie = false;

private:
	float TimerInterval;

	bool bIsTouch; // Is it a touch device
	float FollowTime; // For how long it has been pressed

	
};


