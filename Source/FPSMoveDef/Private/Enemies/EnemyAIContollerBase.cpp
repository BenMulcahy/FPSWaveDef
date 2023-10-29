// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemies/EnemyAIContollerBase.h"
#include "Navigation/CrowdFollowingComponent.h"

AEnemyAIControllerBase::AEnemyAIControllerBase(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer.SetDefaultSubobjectClass<UCrowdFollowingComponent>(TEXT("PathFollowingComponent")))
{
    bStartAILogicOnPossess = true;
}

void AEnemyAIControllerBase::BeginPlay()
{
    Super::BeginPlay();

    RunBehaviorTree(defaultBehaviourTree);
    UE_LOG(LogController, Warning, TEXT("AI Begin Play"));
}
