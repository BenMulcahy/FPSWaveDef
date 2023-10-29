// Fill out your copyright notice in the Description page of Project Settings.

#include "WaveDefGamemode.h"
#include "FPSMoveDef/FPSMoveDef.h"
#include "Framework/EnemySpawnPoint.h"
#include "Framework/WaveDefGameState.h"
#include "Enemies/WaveDefEnemyBase.h"
#include "Kismet/GameplayStatics.h"
#include "..\Public\Character\FPSCharacter.h"
#include "Kismet/KismetMathLibrary.h"
#include "EngineUtils.h"

void AWaveDefGamemode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    GetGameState<AWaveDefGameState>()->players.Add(NewPlayer);

    UE_LOG(LogGameMode, Display, TEXT("Added: %s to Players"), *NewPlayer->GetName());
}





