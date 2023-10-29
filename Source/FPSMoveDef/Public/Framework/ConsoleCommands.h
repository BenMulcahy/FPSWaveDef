// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "ConsoleCommands.generated.h"

/**
 * 
 */
UCLASS()
class FPSMOVEDEF_API UConsoleCommands : public UCheatManager
{
	GENERATED_BODY()

private:
	UFUNCTION(Exec, BlueprintCallable)
		void SetRound(int roundNumber);
		
	UFUNCTION(Exec, BlueprintCallable)
		void GivePoints(int points, int playerID);

	UFUNCTION(Exec, BlueprintCallable)
		void StartGame();

private:
	virtual void InitCheatManager() override;

};
