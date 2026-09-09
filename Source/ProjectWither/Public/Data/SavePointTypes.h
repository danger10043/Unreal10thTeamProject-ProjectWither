#pragma once

#include "CoreMinimal.h"
#include "SavePointTypes.generated.h"

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
};
