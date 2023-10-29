// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "WaveDefGameState.generated.h"

/**
 * Game State should only have values set and read from other places, none of these values should be set in editor -> values that get set live in gameMode 
 */

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoundChanged, int, CurrentRoundNumber);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FGameStarted);

UCLASS()
class FPSMOVEDEF_API AWaveDefGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_RoundChanged)
		int currentRound;

	UPROPERTY(BlueprintAssignable)
		FOnRoundChanged roundChangedDelegate;


	/// <summary>
	/// Broadcast only on server
	/// </summary>
	UPROPERTY(BlueprintAssignable)
		FGameStarted gameStartedDelegate;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Replicated)
		TArray<float> allPlayerPoints;

	//replicating players will return null for any index that isnt the local player 
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Replicated)
		TArray<TObjectPtr<APlayerController>> players;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
		TArray<TObjectPtr<AActor>> spawnPoints;

	UPROPERTY(BlueprintReadOnly)
		bool bGameStarted;

#pragma region Enemy Spawning

	UFUNCTION()
		void SpawnEnemies(const int round);

	UFUNCTION()
		int32 CalculateEnemySpawnCount();

	UFUNCTION()
		float CalculateEnemyHealth();

	UFUNCTION(BlueprintCallable)
		void GetSpawnPoints();

	UFUNCTION(BlueprintCallable)
		void UpdatePlayerPoints();

#pragma endregion

	UFUNCTION(BlueprintCallable)
		bool  HasGameStarted();

private:
	UFUNCTION()
		void OnRep_RoundChanged();

	UPROPERTY()
		TObjectPtr<class AWaveDefGamemode> gameMode;

public:
	UFUNCTION(Server, Reliable)
		virtual void Server_SetCurrentRound(const int roundNumber);
	virtual void Server_SetCurrentRound_Implementation(const int roundNumber);

	UFUNCTION(Server, Reliable)
		void StartGame();
	virtual void StartGame_Implementation();

protected:
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
};
