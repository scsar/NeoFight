#include "SkillCooldownWidget.h"

void USkillCooldownWidget::UpdateSkillCooldown(UProgressBar* ProgressBar, UTextBlock* TextBox, float CooldownTime, float ElapsedTime)
{
    if (ProgressBar)
    {
        float Percent = FMath::Clamp(1.0f - (ElapsedTime / CooldownTime), 0.0f, 1.0f);
        ProgressBar->SetPercent(Percent);
    }

    if (TextBox)
    {
        int RemainingTime = FMath::CeilToInt(CooldownTime - ElapsedTime);
        TextBox->SetText(FText::AsNumber(RemainingTime));
    }
}
