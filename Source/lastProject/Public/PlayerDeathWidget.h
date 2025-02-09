#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerDeathWidget.generated.h"

UCLASS()
class LASTPROJECT_API UPlayerDeathWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	// ���� ���� ��ư
	UPROPERTY(meta = (BindWidget))
	class UButton* QuitButton;

	// ����� ��ư
	UPROPERTY(meta = (BindWidget))
	class UButton* RestartButton;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* GameOverText;

	// �Լ� ���ε�
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnQuitButtonClicked();

	UFUNCTION()
	void OnRestartButtonClicked();
};
