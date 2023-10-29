// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "WaveDefGamemode.generated.h"

/**
 * 
 */

UCLASS()
class FPSMOVEDEF_API AWaveDefGamemode : public AGameModeBase
{
	GENERATED_BODY()

#pragma region Functions
public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TArray<TSubclassOf<class AWaveDefEnemyBase>> enemiesToSpawn;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		float baseEnemyHealth = 150.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		int32 maxEnemiesSpawned = 25;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float maxTimeBetweenEnemySpawns = 2.f;

protected:
	virtual void PostLogin(APlayerController* NewPlayer) override;

#pragma endregion
	
};
