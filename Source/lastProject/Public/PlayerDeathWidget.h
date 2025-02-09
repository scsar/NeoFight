#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerDeathWidget.generated.h"

UCLASS()
class LASTPROJECT_API UPlayerDeathWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	// 게임 종료 버튼
	UPROPERTY(meta = (BindWidget))
	class UButton* QuitButton;

	// 재시작 버튼
	UPROPERTY(meta = (BindWidget))
	class UButton* RestartButton;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* GameOverText;

	// 함수 바인딩
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnQuitButtonClicked();

	UFUNCTION()
	void OnRestartButtonClicked();
};
