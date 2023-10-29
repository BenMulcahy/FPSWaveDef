// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputAction.h"
#include "Components/TimelineComponent.h"
#include "Curves/CurveVector.h"
#include "Character/AdvMovementComponent.h"
#include "FPSUI/UW_GameHUD.h"
#include "Character/FPSPlayerController.h"
#include "FPSCharacter.generated.h"


UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCurrentWeaponChangedDelegate, class ARangedWeapon*, currentWeapon, const class ARangedWeapon*, oldWeapon);

UENUM()
enum class EInteractionType : uint8
{
	null UMETA(DisplayName = "No Interaction"),
	IT_MiscPickup UMETA(DisplayName = "Misc Interact"),
	IT_Weapon UMETA(DisplayName = "Weapon Interact"),
	IT_Door UMETA(DisplayName =  "Door Interact"),
	IT_Modify UMETA(DisplayName = "Modify Interact"),
	IT_Build UMETA(DisplayName = "Build Interact"),
};

UCLASS()
class FPSMOVEDEF_API AFPSCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AFPSCharacter(const FObjectInitializer& object_initializer);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI | HUD")
		TSubclassOf<UUserWidget> HUDWidget;

	UPROPERTY(VisibleInstanceOnly, Category = "UI | HUD")
		class UUW_GameHUD* HUD;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
		class UCameraComponent* camera; 

	UFUNCTION()
		virtual void OnAdvStateUpdate();
	UFUNCTION()
		void CalculateAimSpeed();
	
	/// <summary>
	/// Called whenever current weapon is changed (duh..)
	/// </summary>
	UPROPERTY(BlueprintAssignable, Category = "Delegates")
		FCurrentWeaponChangedDelegate CurrentWeaponChangedDelegate;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		EInteractionType InteractionEnum;

	//weapons array
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "state") //Replicated makes this property replicate over network
	TArray<class ARangedWeapon*> weapons;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "state")
		class AWeaponPickup* weaponPickup;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, ReplicatedUsing = OnRep_CurrentWeapon, Category = "state")
	class ARangedWeapon* CurrentWeapon;

	//current index of weapon
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite,Replicated, Category = "state")
	int32 CurrentIndex = 0;

	/** Movement component used for Advanced movement logic in various movement modes (Sprinting, Crouched, Prone etc), containing relevant settings and functions to control movement. */
	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UAdvMovementComponent> AdvancedMovementComponet;


	//Door Reference
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite)
		class ADoorInteractable* doorInteractable;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void GetValues();
	virtual void SpawnWeapon(class TSubclassOf<ARangedWeapon> weapon);
	virtual void AddToOwnedWeapons(ARangedWeapon* newWeapon = nullptr);
	virtual void ReplaceOwnedWeapon(ARangedWeapon* newWeapon, ARangedWeapon* oldWeapon, int32 currentIndex);
	virtual void RemoveWeapon(ARangedWeapon* weapon);
	virtual void Tick(const float DeltaTime) override;

public:
	virtual void NextWeapon();
	virtual void PreviousWeapon();
	virtual void Jump() override;
	virtual void StopJumping() override;
	virtual void Landed(const FHitResult& Hit) override;
	virtual void ChangeFireMode();
	virtual void ReloadWeapon();

protected:
	UFUNCTION(BlueprintCallable)
		virtual void SetupHUD();

	/// <summary>
	/// Weapon classes spawned by default
	/// </summary>
	UPROPERTY(EditDefaultsOnly, Category = "Weapons | Defaults")
	TArray<TSubclassOf<class ARangedWeapon>> defaultWeapons;

	/// <summary>
	/// Owned Weapon Classes
	/// </summary>
	/// <param name="weapon"></param>]
	UPROPERTY(EditAnywhere, Category = "Weapons | Owned")
		TArray<TSubclassOf<class ARangedWeapon>> owndedWeapons;

	UFUNCTION(Server, Reliable)
		void Server_SetCurrentWeapon(class ARangedWeapon* weapon);
	virtual void Server_SetCurrentWeapon_Implementation(class ARangedWeapon* newWeapon);

	UFUNCTION(Server, Reliable)
		void Server_SpawnWeapon(class TSubclassOf<ARangedWeapon> weapon);
	virtual void Server_SpawnWeapon_Implementation(class TSubclassOf<ARangedWeapon> weapon);

	UFUNCTION(Server, Reliable)
		void Server_TryOpenDoor();
	virtual void Server_TryOpenDoor_Implementation();


public:	

	UFUNCTION(Server, Reliable)
		void Server_AddToLocalPlayerPoints(const float pointsAdded);
	virtual FORCEINLINE void Server_AddToLocalPlayerPoints_Implementation(const float pointsAdded)
	{
		localPlayerPoints += pointsAdded;
	}
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		float maxAimSpeedModifier = 80.f;

	UFUNCTION()
		virtual void OnRep_CurrentWeapon(const class ARangedWeapon* oldWeapon);

	UFUNCTION(BlueprintCallable, Category = "Weapons")
		virtual void EquipWeapon(const int32 index);

	UFUNCTION(BlueprintCallable, Category = "Movement")
		virtual void Sprint();
	
	UFUNCTION(BlueprintCallable, Category = "Movement")
		virtual void DoCrouch();

	UFUNCTION(BlueprintCallable, Category = "Movement")
		virtual void Prone();

	UFUNCTION(BlueprintCallable, Category = "Weapons")
		virtual void StartAim();

	UFUNCTION(BlueprintCallable, Category = "Weapons")
		virtual void StopAim();


	UFUNCTION(BlueprintCallable, Category = "Interaction")
		virtual void Interact();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
		virtual void InteractHold();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons | Animation")
		float adsWeight = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons | Animation")
		float proneWeight = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons | Animation")
		float fovWeight = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons", DisplayName = "Automatic Reload")
		bool bAutoReload = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons | Animation")
		class UCurveVector* defaultWepBob;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons | Equipped")
		int32 maxEquippedWeapons = 2;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Controller")
		TObjectPtr<class AFPSPlayerController> FPSController;

	/// <summary>
	/// Camera Shake
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera | Camera Shake", DisplayName = "Camera Shake Allowed?")
		bool bCanCameraShake = true;

	UFUNCTION(BlueprintCallable)
		virtual void CameraShake(const TSubclassOf<UCameraShakeBase> camShake, const float shakeScale = 1.f);

	float defaultFOV;

#pragma region													AimingStuff
public:
	bool bIsAiming;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Movement | ADS Move Speeds", DisplayName = "Walking ADS Speed")
		float walkADSSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Movement | ADS Move Speeds", DisplayName = "Crouched ADS Speed")
		float crouchedADSSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Movement | ADS Move Speeds", DisplayName = "Proned ADS Speed")
		float proneADSSpeed;

#pragma endregion

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapons | Animation")
		class UCurveFloat* defaultAimCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TObjectPtr<class AWaveDefGameState> waveDefGameState;

	UPROPERTY()
		bool bCanADS = true;

	FTimeline AimingTimeline;

#pragma region ServerLogic

	//Aiming
	UFUNCTION(Server, Unreliable)
		void Server_Aim(const bool bStarting = true);
	virtual FORCEINLINE void Server_Aim_Implementation(const bool bstarting = true)
	{
		MultiAim(bstarting);
		MultiAim_Implementation(bstarting);
	}

	UFUNCTION(NetMulticast, Unreliable)
		void MultiAim(const bool bStarting);
	virtual void MultiAim_Implementation(const bool bStarting);


	UFUNCTION(Server, Unreliable)
		void Server_SetAimSpeed(const float newWalkSpeed);
	virtual FORCEINLINE void Server_SetAimSpeed_Implementation(const float newWalkSpeed)
	{
		AdvancedMovementComponet->MaxWalkSpeed = newWalkSpeed;
	}

#pragma endregion
	UFUNCTION()
		virtual void TimelineProgress(const float value);

public:
	UFUNCTION(BlueprintCallable)
		FORCEINLINE class UAdvMovementComponent* GetAdvancedMovement() const { return AdvancedMovementComponet; }


#pragma region Shooting!!!!!!!!
public:
	UFUNCTION(BlueprintCallable)
		virtual void Shoot();

	UFUNCTION(BlueprintCallable)
		virtual void StopShooting();

	UFUNCTION()
		virtual void ResetShooting();

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Replicated)
		float localPlayerPoints;

protected:
	UFUNCTION()
		void ShootHitscan();


	UFUNCTION(Server, Unreliable)
		void Server_GetHitscanHit(const FVector startPos, const FVector endPos, const float weaponMaxRange);
	virtual void Server_GetHitscanHit_Implementation(const FVector startPos, const FVector endPos, const float weaponMaxRange);

	/// <summary>
	/// Client Side on hit logic
	/// </summary>
	/// <param name="actorHit"> Object Pointer to actor hit </param>
	UFUNCTION(Client, Unreliable)
		void Client_OnHit(const FHitResult actorHit);
	virtual void Client_OnHit_Implementation(const FHitResult actorHit);


	UPROPERTY()
	int32 burstCounter = 0;

bool bIsShooting;

/// <summary>
/// Used to Prevent burst fire stopping shooting premeturely
/// </summary>
bool bSupressStopShoot = false;
bool bSupressStartShot = false;

FTimerHandle shotTimer;
FTimerHandle betweenShotsTimer;


#pragma endregion


};
