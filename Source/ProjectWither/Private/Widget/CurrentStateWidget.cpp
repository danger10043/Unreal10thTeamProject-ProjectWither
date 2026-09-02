#include "Widget/CurrentStateWidget.h"
#include "Components/TextBlock.h"

void UCurrentStateWidget::SetPlayerActionState(EPlayerActionState NewState)
{
	if (!IsValid(PlayerStateText)) return;

	FText StateText = FText::FromString(TEXT("정상"));
	FLinearColor StateColor = FLinearColor::White;

	switch (NewState)
	{
	case EPlayerActionState::AttackingWithSword:
		StateText = FText::FromString(TEXT("근접 공격 중"));
		StateColor = FLinearColor(1.0f, 0.35f, 0.0f, 1.0f);
		break;

	case EPlayerActionState::Rolling:
		StateText = FText::FromString(TEXT("구르기 중"));
		StateColor = FLinearColor(1.0f, 0.85f, 0.0f, 1.0f);
		break;

	case EPlayerActionState::Blocking:
		StateText = FText::FromString(TEXT("방어 중"));
		StateColor = FLinearColor(0.1f, 0.9f, 0.2f, 1.0f);
		break;

	case EPlayerActionState::HitReact:
		StateText = FText::FromString(TEXT("피격 당함"));
		StateColor = FLinearColor(1.0f, 0.25f, 0.25f, 1.0f);
		break;

	case EPlayerActionState::Dead:
		StateText = FText::FromString(TEXT("사망"));
		StateColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);
		break;

	case EPlayerActionState::None:
	default:
		break;
	}

	PlayerStateText->SetText(StateText);
	PlayerStateText->SetColorAndOpacity(FSlateColor(StateColor));
}
