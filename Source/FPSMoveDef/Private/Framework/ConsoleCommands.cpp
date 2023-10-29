// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/ConsoleCommands.h"
#include "Framework/WaveDefGameState.h"
#include "WaveDefGamemode.h"
#include "Character/FPSCharacter.h"


void UConsoleCommands::InitCheatManager()
{
    Super::InitCheatManager();
}

void UConsoleCommands::SetRound(int roundNumber)
{
    if (Cast<AWaveDefGameState>(GetWorld()->GetGameState())) { Cast<AWaveDefGameState>(GetWorld()->GetGameState())->Server_SetCurrentRound(roundNumber); }
}

//TODO: Make work lmao
void UConsoleCommands::GivePoints(int points, int playerID)
{
    APlayerController* PlayerController;
    if (Cast<AWaveDefGameState>(GetWorld()->GetGameState()))
    {
        PlayerController = Cast<AWaveDefGameState>(GetWorld()->GetGameState())->players[playerID].Get();
        if(Cast<AFPSCharacter>(PlayerController->GetCharacter())) Cast<AFPSCharacter>(PlayerController->GetCharacter())->Server_AddToLocalPlayerPoints(points);
    }
}


void UConsoleCommands::StartGame()
{
    if (TObjectPtr<AWaveDefGameState> gs = Cast<AWaveDefGameState>(GetWorld()->GetGameState())) gs->StartGame();
}
