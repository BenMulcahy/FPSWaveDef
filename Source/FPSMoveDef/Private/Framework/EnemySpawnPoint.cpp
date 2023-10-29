// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/EnemySpawnPoint.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AEnemySpawnPoint::AEnemySpawnPoint()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AEnemySpawnPoint::BeginPlay()
{
	Super::BeginPlay();

	//Despawn if this is on the client
	if (GetWorld()->IsNetMode(ENetMode::NM_Client))
	{
		Destroy();
	}
}

FVector AEnemySpawnPoint::SpawnPointLocation()
{
	if (SpawnInRadius)
	{
		//TODO: Sphere Rad Logic
		return GetActorLocation();
	}
	else
	{
		FHitResult hit;
		if (GetWorld()->LineTraceSingleByChannel(hit, GetActorLocation(), (FVector::UpVector * -1 * 100), ECC_Visibility))
		{
			return hit.ImpactPoint;
		}
		else
		{
			//TODO: What shoud this return?
			return GetActorLocation();
		}
	}
}


