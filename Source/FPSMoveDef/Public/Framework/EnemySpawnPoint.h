// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemySpawnPoint.generated.h"

UCLASS()
class FPSMOVEDEF_API AEnemySpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEnemySpawnPoint();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	

	UFUNCTION(BlueprintCallable)
		FVector SpawnPointLocation();


public:

	/** Should spawner spawn enemies at location or in radius around location */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Spawning | Spawn In Radius")
		bool SpawnInRadius = false;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Spawning | Spawn In Radius")
		float SpawnRadius = 20.f;


};
