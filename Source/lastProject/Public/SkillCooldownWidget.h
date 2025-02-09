// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "SkillCooldownWidget.generated.h"

/**
 * 
 */
UCLASS()
class LASTPROJECT_API USkillCooldownWidget : public UUserWidget
{
	GENERATED_BODY()
	

public:
    // 바인딩된 ProgressBars
    UPROPERTY(meta = (BindWidget))
    UProgressBar* SniperSkillProgressBar;

    UPROPERTY(meta = (BindWidget))
    UProgressBar* GatlingSkillProgressBar;

    UPROPERTY(meta = (BindWidget))
    UProgressBar* MineSkillProgressBar;

    UPROPERTY(meta = (BindWidget))
    UProgressBar* BombingSkillProgressBar;

    UPROPERTY(meta = (BindWidget))
    UProgressBar* RollSkillProgressBar;

    // 바인딩된 TextBlocks
    UPROPERTY(meta = (BindWidget))
    UTextBlock* SniperTextBox;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* GatlingTextBox;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* MineTextBox;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* BombingTextBox;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* RollTextBox;

    // 업데이트 함수
    void UpdateSkillCooldown(UProgressBar* ProgressBar, UTextBlock* TextBox, float CooldownTime, float ElapsedTime);
};
