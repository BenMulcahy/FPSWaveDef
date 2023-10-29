// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponPickup.generated.h"

UCLASS()
class FPSMOVEDEF_API AWeaponPickup : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeaponPickup();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TSubclassOf<class ARangedWeapon> weapon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		class UStaticMeshComponent* weaponMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		class UDecalComponent* weaponDecal;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class USceneComponent* root;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		class UBoxComponent* interactionBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float weaponCost = 100.f;

	//On Begin overlap
	UFUNCTION()
		void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	//On End Overlap
	UFUNCTION()
		void OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

protected:
	virtual void BeginPlay() override;
};
