// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHUDWidget.generated.h"

/**
 * 
 */
UCLASS()
class LASTPROJECT_API UPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
    /** HP Progress Bar 업데이트 함수 */
    UFUNCTION(BlueprintCallable, Category = HUD)
    void UpdateHPBar(float CurrentHP, float MaxHP);

protected:
    UPROPERTY(meta = (BindWidget))
    class UProgressBar* HPProgressBar;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* HPText;

};
