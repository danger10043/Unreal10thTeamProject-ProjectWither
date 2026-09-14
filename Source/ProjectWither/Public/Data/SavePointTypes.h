#pragma once

#include "CoreMinimal.h"
#include "SavePointTypes.generated.h"

class UTexture2D;

// UI 표시용 세이브 포인트 정보 (패스트 트래블 목록 등에서 사용)
USTRUCT(BlueprintType)
struct FSavePointInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "SavePoint")
	FName SavePointId;

	UPROPERTY(BlueprintReadOnly, Category = "SavePoint")
	FText DisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "SavePoint")
	FVector Location = FVector::ZeroVector;

	// 현재 부활 지점으로 지정된 세이브 포인트인지 여부
	UPROPERTY(BlueprintReadOnly, Category = "SavePoint")
	bool bIsCurrent = false;

	// 빠른 이동 목록에서 이 항목에 마우스를 올렸을 때 보여줄 장소 이미지
	UPROPERTY(BlueprintReadOnly, Category = "SavePoint")
	TObjectPtr<UTexture2D> LocationImage = nullptr;
};
