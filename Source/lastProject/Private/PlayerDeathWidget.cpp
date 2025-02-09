#include "PlayerDeathWidget.h"
#include "Components/Button.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"

void UPlayerDeathWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Quit ��ư Ŭ�� �� �Լ� ���ε�
	if (QuitButton)
	{
		QuitButton->OnClicked.AddDynamic(this, &UPlayerDeathWidget::OnQuitButtonClicked);
	}

	// Restart ��ư Ŭ�� �� �Լ� ���ε�
	if (RestartButton)
	{
		RestartButton->OnClicked.AddDynamic(this, &UPlayerDeathWidget::OnRestartButtonClicked);
	}
}

void UPlayerDeathWidget::OnQuitButtonClicked()
{
	// ���� ����
	UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, false);
}

void UPlayerDeathWidget::OnRestartButtonClicked()
{
	// ���� ���� �����
	UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()), false);
}
