// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "WaveDefPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class FPSMOVEDEF_API AWaveDefPlayerState : public APlayerState
{
	GENERATED_BODY()
	

public:

	UFUNCTION(Client, Unreliable)
		void UpdateClientOtherPlayersPoints();
	virtual void UpdateClientOtherPlayersPoints_Implementation();

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
		TArray<float> otherPlayerPoints;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
		virtual void OnGameStart();

	UPROPERTY()
		TObjectPtr<class AWaveDefGameState> gameState;

private:
	FTimerHandle pTimer;
};
