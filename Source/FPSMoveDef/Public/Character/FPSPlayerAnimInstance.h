// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Weapons/RangedWeapon.h"
#include "FPSPlayerAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class FPSMOVEDEF_API UFPSPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	//Constructor
	UFPSPlayerAnimInstance();

protected:
	//Anim blueprint begin play 
	virtual void NativeBeginPlay() override;

	//Tick for updating animation
	virtual void NativeUpdateAnimation(float DeltaTime) override;

	UFUNCTION()
	virtual void CurrentWeaponChanged(class ARangedWeapon* newWep, const class ARangedWeapon* oldWep);

	virtual void SetVars(const float DeltaTime);
	virtual void CalculateWeaponSway(const float DeltaTime);
	virtual void SetIKTransforms();

public:
	UPROPERTY(BlueprintReadWrite, Category = "Animation")
		class AFPSCharacter* character;

	UPROPERTY(BlueprintReadWrite, Category = "Animaion")
		class USkeletalMeshComponent* mesh;

	UPROPERTY(BlueprintReadWrite, Category = "Animation")
		class ARangedWeapon* currentWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
		float adsWeight = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
		float proneWeight = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
		FIKProperties ikProperties;
		
#pragma region ikVars

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
		FTransform camTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
		FTransform relCamTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
		FTransform rHandToSightsTrans;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
		FTransform swayOffsetTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
		float swaySpeed = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
		float swayToZeroSpeed = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
		float weaponBopSpeed = 10.f;

#pragma endregion


#pragma region WeaponSway Offsets
	FRotator AccumulativeRotation;
	FRotator AccumulativeRotInterp;
	FRotator LastRotation;
	FVector AccumulativeLocation;
	FVector AccumulativeLocationInterp;

#pragma endregion

};
