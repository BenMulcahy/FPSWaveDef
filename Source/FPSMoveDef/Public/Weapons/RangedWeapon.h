// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraComponent.h"
#include "Components/AudioComponent.h"
#include "RangedWeapon.generated.h"

UENUM()
enum class EProjectileType : uint8
{
	PT_HitScan UMETA(DisplayName = "Hit Scan"), PT_Projectile UMETA(DisplayName = "Projectile"), PT_Custom UMETA(DisplayName = "Custom")
};

UENUM(BlueprintType)
enum class EFireMode : uint8
{
	FM_SingleFire UMETA(DisplayName = "Single Fire"), FM_BurstFire UMETA(DisplayName = "Burst Fire"), FM_FullAutoFire UMETA(DisplayName = "Fully Automatic Fire")
};

USTRUCT(BlueprintType)
struct FIKProperties
{
	GENERATED_BODY()

	/// <summary>
	/// Base animation pose
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UAnimSequence* animPose;

	//Pose for Prone
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class UAnimSequence* pronePose;

	/// <summary>
	/// Distance between camera and sights when aiming
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float AimOffset = 15.f;

	/// <summary>
	/// Offset from base pose to where the weapon is
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform CustomOffsetTransform;
	/// <summary>
	/// Amount of WeaponSway
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
		float weaponSwayScale = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
		float adsWeaponSwayScale = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
		FVector weaponBobScale = FVector(0.f,1.f,1.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
		FVector adsWeaponBobScale = FVector(0.f, 0.2f, 0.2f);

};

UCLASS(Abstract) //Abstract bc we arent referincing A weapon we will only want subclasses 
class FPSMOVEDEF_API ARangedWeapon : public AActor
{ 
	GENERATED_BODY()
	
public:
	//Constructor
	ARangedWeapon();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
		class USceneComponent* root;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Mesh")
		class USkeletalMeshComponent* weaponMesh;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "state")
		class AFPSCharacter* currentOwner;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Config | Weapon Data")
		EProjectileType projectileType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Config | Weapon Data")
		EFireMode defaultFireMode;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Config | Weapon Data", DisplayName = "Usable Fire Modes")
		TArray<EFireMode> fireModes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Config | Weapon Data", DisplayName = "Time between each shot/burst", meta = (ClampMin = "0", UIMin = "0"))
		float timebetweenSingleShots = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config | Weapon Data",DisplayName = "Fire Rate", meta = (ClampMin = "1", UIMin = "1"))
		int32 fireRate = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config | Weapon Data", DisplayName = "Shots per Burst", meta = (ClampMin = "1", UIMin = "1"))
		int32 burstCount = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config | Weapon Data", DisplayName = "Burst Fire Rate", meta = (ClampMin = "1", UIMin = "1"))
		int32 burstFireRate = 15;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config | Weapon Data", DisplayName = "Damage Per Bullet", meta = (ClampMin = "0", UIMin = "0"))
		float damagePerBullet = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Config | Weapon Data")
		TObjectPtr<UCurveBase> rangeCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config | Weapon Data", DisplayName = "Magazine Size", meta = (ClampMin = "1", UIMin = "1"))
		int32 magSize = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config | Weapon Data", DisplayName = "Reserve Ammo", meta = (ClampMin = "0", UIMin = "0"))
		int32 maxReserveAmmo = 125;

	/// <summary>
	/// Higher Values result in more shot deviation from centre screen ( 0 = 100% Accuracy )
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config | Weapon Data", DisplayName = "Weapon Shot Deviation", meta = (ClampMin = "0", UIMin = "0"))
		float weaponAccuracy = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config | Weapon Data", DisplayName = "Time To Reload", meta = (ClampMin = "0.001", UIMin = "0.001"))
		float timeToReload = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config | Weapon Data")
		 float ADSFov = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config | Weapon Data")
		float moveSpeedModifier = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config | Anim Data")
		FIKProperties ikProperties;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config | Anim Data")
		FTransform PlacementTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config | FX | VFX")
		TObjectPtr<UNiagaraSystem> muzzleFlash;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config | FX | Audio")
		TObjectPtr<USoundBase> shotSFX;

	// Shouldnt need to be changed as all weapons should have the same points per hit but allows for overide.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config | Weapon Data | Points")
		float pointPerHit = 10.f;

	// Shouldnt need to be changed as all weapons should have the same points per hit but allows for overide.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config | Weapon Data | Points")
		float pointPerHitHeadshot = 50.f;

	/// <summary>
	/// Time to ADS
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config | Anim Data", DisplayName = "Time to ADS Scalar")
		float ADSSpeedScalar = 0.2f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		EFireMode currentFireMode;

	/// <summary>
	/// How many bullets left in the magazine
	/// </summary>
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
		int32 currentBulletCount;


	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
		int32 currentReserveCount;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "IK") //Blueprint native = 2 implementations of C++ default and blueprint override 
		FTransform GetSightsWorldTransform() const;
	virtual FORCEINLINE FTransform GetSightsWorldTransform_Implementation() const {return weaponMesh->GetSocketTransform(FName("Sights")); }

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "IK")
		FTransform GetMuzzleWorldTransform() const;
	virtual FORCEINLINE FTransform GetMuzzleWorldTransform_Implementation() const { return weaponMesh->GetSocketTransform(FName("Muzzle")); }

	UFUNCTION(BlueprintCallable)
		void SwitchFireMode();

	UFUNCTION(BlueprintCallable)
		void PlayMuzzleFlash();

	UFUNCTION(Server, Unreliable)
		void Server_PlayFX();
	virtual FORCEINLINE void Server_PlayFX_Implementation()
	{
		//UE_LOG(LogTemp, Error, TEXT("Server Play FX!"));
		MultiPlayWeaponFX();
	}

	UFUNCTION(NetMultiCast, Unreliable)
		void MultiPlayWeaponFX();
	virtual void MultiPlayWeaponFX_Implementation();

	UFUNCTION(BlueprintCallable)
		void PlayShotSound();

	UFUNCTION()
		bool UpdateAmmo();

	UFUNCTION(BlueprintCallable)
		virtual void Reload();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(const float DeltaTime) override;

	UPROPERTY(BlueprintReadOnly, Category = "Config | FX | Audio")
		TObjectPtr<UAudioComponent> weaponAudioComponent;

	UPROPERTY(BlueprintReadOnly, Category = "Config | FX | Audio")
		TObjectPtr<UNiagaraComponent> weaponNiagaraComponent;
private:
	UFUNCTION()
		void DoReload();

	FTimerHandle reloadTimer;
	bool bIsReloading;
};
