#include "PlayerHUDWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UPlayerHUDWidget::UpdateHPBar(float CurrentHP, float MaxHP)
{
    if (HPProgressBar)
    {
        HPProgressBar->SetPercent(CurrentHP / MaxHP);
    }

    if (HPText)
    {
        FText HPTextValue = FText::FromString(FString::Printf(TEXT("%d / %d"), FMath::FloorToInt(CurrentHP), FMath::FloorToInt(MaxHP)));
        HPText->SetText(HPTextValue);
    }
}
