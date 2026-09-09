// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/SubSystem/SavePointSubsystem.h"

void USavePointSubsystem::RegisterSavePoint(FName SavePointId, const FText& DisplayName, const FTransform& Transform)
{
	if (SavePointId.IsNone()) return;

	FSavePointRecord& Record = SavePoints.FindOrAdd(SavePointId);
	Record.DisplayName = DisplayName;
	Record.Transform = Transform;
}

bool USavePointSubsystem::ActivateSavePoint(FName SavePointId)
{
	FSavePointRecord* Record = SavePoints.Find(SavePointId);

	if (Record == nullptr) return false;

	Record->bActivated = true;
	CurrentSavePointId = SavePointId;

	return true;
}

bool USavePointSubsystem::IsSavePointActivated(FName SavePointId) const
{
	const FSavePointRecord* Record = SavePoints.Find(SavePointId);

	return Record != nullptr && Record->bActivated;
}

bool USavePointSubsystem::HasCurrentSavePoint() const
{
	return IsSavePointActivated(CurrentSavePointId);
}

FTransform USavePointSubsystem::GetCurrentSavePointTransform() const
{
	FTransform OutTransform;

	if (GetSavePointTransform(CurrentSavePointId, OutTransform))
	{
		return OutTransform;
	}

	return FTransform::Identity;
}

bool USavePointSubsystem::GetSavePointTransform(FName SavePointId, FTransform& OutTransform) const
{
	const FSavePointRecord* Record = SavePoints.Find(SavePointId);

	if (Record == nullptr) return false;

	OutTransform = Record->Transform;

	return true;
}

TArray<FSavePointInfo> USavePointSubsystem::GetActivatedSavePoints() const
{
	TArray<FSavePointInfo> Result;

	for (const TPair<FName, FSavePointRecord>& Pair : SavePoints)
	{
		if (!Pair.Value.bActivated) continue;

		FSavePointInfo Info;
		Info.SavePointId = Pair.Key;
		Info.DisplayName = Pair.Value.DisplayName;
		Info.Location = Pair.Value.Transform.GetLocation();
		Info.bIsCurrent = Pair.Key == CurrentSavePointId;

		Result.Add(Info);
	}

	return Result;
}
