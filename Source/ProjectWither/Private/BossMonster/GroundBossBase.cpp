// Fill out your copyright notice in the Description page of Project Settings.


#include "BossMonster/GroundBossBase.h"
#include "Component/BossComponent.h"

AGroundBossBase::AGroundBossBase()
{
    BossComponent = CreateDefaultSubobject<UBossComponent>(TEXT("BossComponent"));
}
