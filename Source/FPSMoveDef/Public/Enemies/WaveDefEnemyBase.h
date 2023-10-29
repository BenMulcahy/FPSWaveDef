// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "WaveDefEnemyBase.generated.h"

UCLASS()
class FPSMOVEDEF_API AWaveDefEnemyBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AWaveDefEnemyBase();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
		TObjectPtr<class UHealthComponent> healthComponent;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
