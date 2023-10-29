// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/WaveDefPlayerState.h"
#include "Framework/WaveDefGameState.h"

void AWaveDefPlayerState::UpdateClientOtherPlayersPoints_Implementation()
{
    if (gameState->allPlayerPoints.IsEmpty()) { UE_LOG(LogGameState, Error, TEXT("No Players points for client!")); return; }

    //UE_LOG(LogTemp, Display, TEXT("Client update points"));

    TArray<float> tmp = gameState->allPlayerPoints;

    for (int32 i = 0; i < gameState->players.Num(); i++)
    {
        //only true for local player
        if (gameState->players[i] != nullptr)
        {
            tmp.RemoveAt(i); //discard local players points
        }
    }

    otherPlayerPoints = tmp; //save all players points without local player
}

void AWaveDefPlayerState::BeginPlay()
{
    Super::BeginPlay();

    gameState = GetWorld()->GetGameState<AWaveDefGameState>();
    if (gameState)
    {
        UE_LOG(LogTemp, Display, TEXT("Delegate set"));
        gameState->gameStartedDelegate.AddDynamic(this, &AWaveDefPlayerState::OnGameStart);
    }
}

void AWaveDefPlayerState::OnGameStart()
{
    UE_LOG(LogTemp, Warning, TEXT("Game Started!!!!"));
    GetWorld()->GetTimerManager().SetTimer(pTimer, this, &AWaveDefPlayerState::UpdateClientOtherPlayersPoints, 0.35f, true);
}
