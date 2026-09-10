#include "Widget/SegmentedStatBarWidget.h"

#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Widget/StatBarSegmentWidget.h"

void USegmentedStatBarWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	RefreshSegments(true);
}

void USegmentedStatBarWidget::SetValues(
	float CurrentValue,
	float MaxValue
)
{
	CachedMaxValue = FMath::Max(0.0f, MaxValue);
	CachedCurrentValue = FMath::Clamp(
		CurrentValue,
		0.0f,
		CachedMaxValue
	);

	RefreshSegments(false);
}

void USegmentedStatBarWidget::RefreshSegments(bool bForceRebuild)
{
	if (!IsValid(SegmentContainer)) return;

	const float MaxValue = IsDesignTime()
		? FMath::Max(0.0f, PreviewMaxValue)
		: CachedMaxValue;

	const float CurrentValue = IsDesignTime()
		? FMath::Clamp(PreviewCurrentValue, 0.0f, MaxValue)
		: CachedCurrentValue;

	const int32 TotalCount = FMath::CeilToInt(
		MaxValue / ValuePerSegment
	);

	const int32 OnCount = FMath::Clamp(
		FMath::CeilToInt(CurrentValue / ValuePerSegment),
		0,
		TotalCount
	);

	const bool bNeedsRebuild =
		bForceRebuild ||
		Segments.Num() != TotalCount ||
		SegmentContainer->GetChildrenCount() != TotalCount;

	if (bNeedsRebuild)
	{
		SegmentContainer->ClearChildren();
		Segments.Reset();

		if (TotalCount <= 0) return;

		if (!SegmentWidgetClass)
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("SegmentedStatBarWidget - SegmentWidgetClass가 지정되지 않았습니다.")
			);
			return;
		}

		Segments.Reserve(TotalCount);

		for (int32 Index = 0; Index < TotalCount; ++Index)
		{
			UStatBarSegmentWidget* Segment =
				CreateWidget<UStatBarSegmentWidget>(
					this,
					SegmentWidgetClass
				);

			if (!IsValid(Segment))
			{
				SegmentContainer->ClearChildren();
				Segments.Reset();

				UE_LOG(
					LogTemp,
					Warning,
					TEXT("SegmentedStatBarWidget - Segment 생성에 실패했습니다.")
				);
				return;
			}

			// 컨테이너에 붙이기 전에 표시값을 설정한다.
			Segment->SetAppearance(
				Index < OnCount ? BarOn : BarOff,
				SegmentSize
			);

			UHorizontalBoxSlot* SegmentSlot =
				SegmentContainer->AddChildToHorizontalBox(Segment);

			if (IsValid(SegmentSlot))
			{
				SegmentSlot->SetSize(
					FSlateChildSize(ESlateSizeRule::Automatic)
				);
				SegmentSlot->SetHorizontalAlignment(HAlign_Fill);
				SegmentSlot->SetVerticalAlignment(VAlign_Center);

				const float RightPadding = Index < TotalCount - 1
					? FMath::Max(0.0f, SegmentSpacing)
					: 0.0f;

				SegmentSlot->SetPadding(
					FMargin(0.0f, 0.0f, RightPadding, 0.0f)
				);
			}

			Segments.Add(Segment);
		}

		return;
	}

	// 현재 값만 바뀌었다면 기존 칸의 이미지만 갱신한다.
	for (int32 Index = 0; Index < Segments.Num(); ++Index)
	{
		if (!IsValid(Segments[Index])) continue;

		Segments[Index]->SetAppearance(
			Index < OnCount ? BarOn : BarOff,
			SegmentSize
		);
	}
}
// + + < 코드 추가 종료 > + +