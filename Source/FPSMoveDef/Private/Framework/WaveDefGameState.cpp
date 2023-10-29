// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/WaveDefGameState.h"
#include "Enemies/WaveDefEnemyBase.h"
#include "Framework/EnemySpawnPoint.h"
#include "WaveDefGamemode.h"
#include "..\..\Public\Character\FPSCharacter.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"


//TODO:
// Keep on makin points system
//TODO: Make spawns happen over time

bool AWaveDefGameState::HasGameStarted()
{
    if (bGameStarted) return true;
    else return false;
}

void AWaveDefGameState::OnRep_RoundChanged()
{
    UE_LOG(LogGameState, Display, TEXT("Set Round to: %d"), currentRound);

    roundChangedDelegate.Broadcast(currentRound);
}

void AWaveDefGameState::Server_SetCurrentRound_Implementation(const int roundNumber)
{
    if (!HasAuthority()) return; //Just in case client somehow calls this
    currentRound = roundNumber;
    SpawnEnemies(roundNumber);
}

void AWaveDefGameState::SpawnEnemies(const int round)
{
    if (!gameMode) { UE_LOG(LogGameState, Error, TEXT("No Game Mode found when spawning enemies")); return; }

    GetSpawnPoints();
    if (spawnPoints.Num() <= 0)
    {
        UE_LOG(LogGameState, Error, TEXT("No Spawn points found, Spawning wont be done"));
        return;
    }

    if (gameMode->enemiesToSpawn.Num() <= 0)
    {
        UE_LOG(LogGameState, Error, TEXT("No Enemies to spawn!"));
        return;
    }

    int enemyNum = CalculateEnemySpawnCount();
    float enemyHealth = CalculateEnemyHealth();

    UE_LOG(LogGameState, Warning, TEXT("Spawning %d enemies with %f health"), enemyNum, enemyHealth);

    for (int32 i = 0; i < enemyNum; i++)
    {
        int32 point = UKismetMathLibrary::RandomIntegerInRange(0, spawnPoints.Num() - 1);
        int32 enemy = UKismetMathLibrary::RandomIntegerInRange(0, gameMode->enemiesToSpawn.Num() - 1);

        FActorSpawnParameters params;
        GetWorld()->SpawnActorAbsolute<AWaveDefEnemyBase>(gameMode->enemiesToSpawn[enemy], spawnPoints[point]->GetTransform(), params);
        UE_LOG(LogGameState, Display, TEXT("Spawn Enemy @ %s"), *spawnPoints[point].GetName());
    }
}

int32 AWaveDefGameState::CalculateEnemySpawnCount()
{
    const int32 playersInGame = players.Num();
    //Formulas dont work before r12 so hardcode :D

    if (playersInGame == 1)
    {
        static const TArray<int32> preRound12 = { 6,8,13,18,24,27,28,29,33,34 };

        if (preRound12.IsValidIndex(currentRound - 1))
        {
            return preRound12[currentRound - 1];
        }
    }

    if (playersInGame == 2)
    {
        static const TArray<int32> preRound12 = { 8,10,19,24,30,33,35,36,39,40 };

        if (preRound12.IsValidIndex(currentRound - 1))
        {
            return preRound12[currentRound - 1];
        }
    }

    if (playersInGame == 3)
    {
        static const TArray<int32> preRound12 = { 10,12,21,27,32,35,37,38,40,41 };

        if (preRound12.IsValidIndex(currentRound - 1))
        {
            return preRound12[currentRound - 1];
        }
    }

    if (playersInGame == 4)
    {
        static const TArray<int32> preRound12 = { 10,12,21,27,32,35,37,38,40,41 };

        if (preRound12.IsValidIndex(currentRound - 1))
        {
            return preRound12[currentRound - 1];
        }
    }

    //No idea just stolen from zombies 
    switch (playersInGame)
    {
    case 1:
        return FMath::FloorToInt(0.000058f * FMath::Pow(currentRound, 3.f) + 0.074032f * FMath::Pow(currentRound, 2.f) + 0.718119f * currentRound + 14.738699f);
        break;
    case 2:
        return FMath::FloorToInt(0.000054f * FMath::Pow(currentRound, 3.f) + 0.169717f * FMath::Pow(currentRound, 2.f) + 0.541627f * currentRound + 15.917041f);
        break;

    case 3:
        return FMath::FloorToInt(0.000169f * FMath::Pow(currentRound, 3.f) + 0.238079f * FMath::Pow(currentRound, 2.f) + 1.307276f * currentRound + 21.291046f);
        break;

    case 4:
        return FMath::FloorToInt(0.000225f * FMath::Pow(currentRound, 3.f) + 0.314314f * FMath::Pow(currentRound, 2.f) + 1.835712f * currentRound + 27.596132f);
        break;
    default:
        UE_LOG(LogGameMode, Error, TEXT("No Players/Players out of range? Out of Range?"));
        return 0;
        break;
    }


}

float AWaveDefGameState::CalculateEnemyHealth()
{
    float health = gameMode->baseEnemyHealth;
    health += FMath::Min(8, currentRound - 1) * 100;

    if (currentRound >= 10)
    {
        health *= FMath::Pow(1.1f, currentRound - 9);
    }

    return health;
}

void AWaveDefGameState::GetSpawnPoints()
{
    spawnPoints.Empty();
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemySpawnPoint::StaticClass(), spawnPoints);
    UE_LOG(LogGameState, Display, TEXT("Found: %d enemy spawn points"), spawnPoints.Num());

    if (spawnPoints.Num() <= 0) UE_LOG(LogGameState, Error, TEXT("No Spawn points found!"));
}

void AWaveDefGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AWaveDefGameState, currentRound);
    DOREPLIFETIME(AWaveDefGameState, allPlayerPoints);
    DOREPLIFETIME(AWaveDefGameState, players);
}

void AWaveDefGameState::UpdatePlayerPoints()
{
    if (!HasAuthority()) return;
    //UE_LOG(LogGameMode, Display, TEXT("Attempting to update player points"));
    for (int32 i = 0; i < players.Num(); i++)
    {
        //UE_LOG(LogGameMode, Warning, TEXT("Casting to player"));
        if (AFPSCharacter* player = Cast<AFPSCharacter>(players[i]->GetCharacter()))
        {
            UE_LOG(LogGameState, Display, TEXT("Updating Player points"));
            //if already has value in points i
            if (allPlayerPoints.IsValidIndex(i))
            {
                allPlayerPoints[i] = player->localPlayerPoints;
            }
            //if not valid index (player doesnt have a points val
            else
            {
                UE_LOG(LogGameState, Warning, TEXT("Player points non-existant!"));
                allPlayerPoints.Insert(player->localPlayerPoints, i); //Can (and proably will) cause issues later down the line TODO!!
            }
        }
    }
}

void AWaveDefGameState::BeginPlay()
{
    //Get the game mode
    gameMode = Cast<AWaveDefGamemode>(AuthorityGameMode);
    GetSpawnPoints();
}

void AWaveDefGameState::StartGame_Implementation()
{
    if (!HasAuthority()) return; //Just in case client somehow calls this
    UE_LOG(LogGameState, Display, TEXT("Starting Game..."));
    FTimerHandle pointsTimer;

    //Start the game
    Server_SetCurrentRound(1);
    GetWorld()->GetTimerManager().SetTimer(pointsTimer, this, &AWaveDefGameState::UpdatePlayerPoints, 0.35f, true);

    bGameStarted = true;
    gameStartedDelegate.Broadcast();
}

