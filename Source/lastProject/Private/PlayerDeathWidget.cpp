#include "PlayerDeathWidget.h"
#include "Components/Button.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"

void UPlayerDeathWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Quit 버튼 클릭 시 함수 바인딩
	if (QuitButton)
	{
		QuitButton->OnClicked.AddDynamic(this, &UPlayerDeathWidget::OnQuitButtonClicked);
	}

	// Restart 버튼 클릭 시 함수 바인딩
	if (RestartButton)
	{
		RestartButton->OnClicked.AddDynamic(this, &UPlayerDeathWidget::OnRestartButtonClicked);
	}
}

void UPlayerDeathWidget::OnQuitButtonClicked()
{
	// 게임 종료
	UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, false);
}

void UPlayerDeathWidget::OnRestartButtonClicked()
{
	// 현재 레벨 재시작
	UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()), false);
}
