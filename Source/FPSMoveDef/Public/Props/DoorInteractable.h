// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DoorInteractable.generated.h"

UCLASS()
class FPSMOVEDEF_API ADoorInteractable : public AActor
{
	GENERATED_BODY()

// ------------------------------- FUNCTIONS
	
public:
	// Sets default values for this actor's properties
	ADoorInteractable();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void OpenDoor();
	virtual void OpenDoor_Implementation();
	
protected:
	//On Begin overlap
	UFUNCTION()
	void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	//On End Overlap
	UFUNCTION()
	void OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	
// ------------------------------- Variables

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "state")
		bool bCanOpen = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "state")
		FRotator DoorOpenedRotation = FRotator(0.f,-130.f,0.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "points")
		int DoorCost;
	
protected:

	UPROPERTY()
		USceneComponent* root;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Door mesh")
		UStaticMeshComponent* DoorMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Door mesh")
		UStaticMeshComponent* DoorFrameMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	class UBoxComponent* interactionBox;

private:
	bool bOpened = false; 
};
