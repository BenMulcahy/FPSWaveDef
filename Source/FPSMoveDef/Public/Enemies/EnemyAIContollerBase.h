// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIContollerBase.generated.h"

/**
 * 
 */
UCLASS()
class FPSMOVEDEF_API AEnemyAIControllerBase : public AAIController
{
	GENERATED_BODY()
	
public:
	AEnemyAIControllerBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behaviour Tree")
		TObjectPtr<class UBehaviorTree> defaultBehaviourTree;

protected:
	virtual void BeginPlay() override;
};
