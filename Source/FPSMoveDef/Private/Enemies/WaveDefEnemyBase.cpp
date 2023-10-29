// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemies/WaveDefEnemyBase.h"
#include "Enemies/EnemyAIContollerBase.h"
#include "Net/UnrealNetwork.h"
#include "HealthComponent.h"

// Sets default values
AWaveDefEnemyBase::AWaveDefEnemyBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//AI Pawn setup default vals
	AutoPossessPlayer = EAutoReceiveInput::Disabled; 
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	healthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("Health Component"));
}

// Called when the game starts or when spawned
void AWaveDefEnemyBase::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AWaveDefEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}

