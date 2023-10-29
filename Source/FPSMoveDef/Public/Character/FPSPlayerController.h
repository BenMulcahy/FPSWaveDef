// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FPSPlayerController.generated.h"

/**
 * 
 */

//TODO: Move player points to gamestate and store each player all in one place

class UInputAction;

UCLASS()
class FPSMOVEDEF_API AFPSPlayerController : public APlayerController
{
	GENERATED_BODY()


public:
	AFPSPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
		TObjectPtr<class AFPSCharacter> playerCharacter;

public:
	
	UFUNCTION(BlueprintCallable)
		void CallJump();

	UFUNCTION(BlueprintCallable)
		void CallStopJump();

	UFUNCTION(BlueprintCallable)
		void CallNextWeapon();

	UFUNCTION(BlueprintCallable)
		void CallPreviousWeapon();

	UFUNCTION(BlueprintCallable)
		void CallStartAim();

	UFUNCTION(BlueprintCallable)
		void CallStopAim();

	UFUNCTION(BlueprintCallable)
		void CallShoot();

	UFUNCTION(BlueprintCallable)
		void CallStopShoot();

	UFUNCTION(BlueprintCallable)
		void CallChangeFireMode();

	UFUNCTION(BlueprintCallable)
		void CallReload();

	UFUNCTION(BlueprintCallable)
		void CallCrouch();

	UFUNCTION(BlueprintCallable)
		void CallInteract();

	UFUNCTION(BlueprintCallable)
		void CallInteractHeld();

	UFUNCTION(BlueprintCallable)
		void CallProne();

	UFUNCTION(BlueprintCallable)
		void CallSprint();

protected:

	UFUNCTION(BlueprintCallable)
		void Move(const FInputActionValue& Value);

	UFUNCTION(BlueprintCallable)
		void Look(const FInputActionValue& Value);

public:
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		class UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> MoveAction;

	//Sprint
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> SprintAction;

	/** Crouch Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> CrouchAction;

	/** Prone Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> ProneAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> LookAction;

	//Next Weap
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> NextWeaponAction;

	//Last Weap
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> LastWeaponAction;

	//ADS
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> AimDownSights;

	//Pickup
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> InteractAction;

	//Shoot
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> shootAction;

	//Change Fire Type
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> changeFireAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> reloadAction;
};
