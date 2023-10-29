// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/FPSCharacter.h"
#include "Character/FPSPlayerController.h"
#include "Weapons/RangedWeapon.h"
#include "Weapons/WeaponPickup.h"
#include "FPSMoveDef/FPSMoveDef.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "HealthComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetMathLibrary.h"
#include "Blueprint/UserWidget.h"
#include "FPSUI/UW_GameHUD.h"
#include "Character/AdvMovementComponent.h"
#include "Props/DoorInteractable.h"

//TODO:
//Why controller change weapon crash if hit once when only 1 wep equipped and then interact with wall buy?
//Add headshot multiplyer etc

// Sets default values
AFPSCharacter::AFPSCharacter(const FObjectInitializer& object_initializer) : Super(object_initializer.SetDefaultSubobjectClass<UAdvMovementComponent>(AFPSCharacter::CharacterMovementComponentName))
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	

	GetMesh()->SetTickGroup(TG_PostUpdateWork);
	GetMesh()->bVisibleInRayTracing = true;
	GetMesh()->bVisibleInReflectionCaptures = true;
	GetMesh()->bCastHiddenShadow = true;

	AdvancedMovementComponet = Cast<UAdvMovementComponent>(Super::GetMovementComponent());

	camera = CreateDefaultSubobject<UCameraComponent>(TEXT("fpsCamera"));
	camera->bUsePawnControlRotation = true;
	camera->SetupAttachment(GetMesh(), FName("head")); //Attach camera to head socket
}


// Called when the game starts or when spawned
void AFPSCharacter::BeginPlay()
{
	Super::BeginPlay();

	if(IsLocallyControlled() || HasAuthority())
	{
		//Logic for Server/Local Player
		GetValues();
	}
	if (!HasAuthority())
	{
		//Logic for other clients
	}



#pragma region Weapon Logic
	//Creating ADS timeline
	if (defaultAimCurve)
	{
		FOnTimelineFloat TimelineFloat;
		TimelineFloat.BindDynamic(this, &AFPSCharacter::TimelineProgress);

		AimingTimeline.AddInterpFloat(defaultAimCurve,TimelineFloat);
	}

	
	//Spawning Default Weapons
	if (HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: Spawn Default Weapons with Authority"), *this->GetName());
		for (const TSubclassOf<ARangedWeapon>& weaponToSpawn : defaultWeapons) //iterate over all default wpns
		{
			if (!weaponToSpawn) continue; //if not valid just skip
			FActorSpawnParameters params;
			params.Owner = this;
			ARangedWeapon* spawnedWeapon = GetWorld()->SpawnActor<ARangedWeapon>(weaponToSpawn, params);
			AddToOwnedWeapons(spawnedWeapon);
		}
	}
	
#pragma endregion

	//Only if local player
	if (IsLocallyControlled()) 
	{
		SetupHUD();
	}
}

void AFPSCharacter::SetupHUD()
{
	if (IsValid(HUDWidget))
	{
		HUD = Cast<UUW_GameHUD>(CreateWidget(GetWorld(), HUDWidget));
		
		if (HUD)
		{
			if(Cast<AFPSPlayerController>(FPSController)) HUD->playerController = FPSController;
			HUD->fpsPlayer = this;
			if(CurrentWeapon) HUD->currentWeaponEquipped = CurrentWeapon;
			HUD->AddToViewport();
		}
	}
}

void AFPSCharacter::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);
	AimingTimeline.TickTimeline(DeltaTime);

	if (defaultAimCurve)
	{
		camera->SetFieldOfView(FMath::Lerp(defaultFOV, CurrentWeapon ? CurrentWeapon->ADSFov : defaultFOV, fovWeight));
	}

#pragma region Debugging


	//OnScreenDebug
	FString wepName;
	if (weaponPickup) wepName = weaponPickup->GetName();
	else wepName = "Null";

	//	Server and Client Weapon Lists
	/*
	if (HasAuthority() && GEngine) {
		for (int16 i = 0; i < weapons.Num(); i++)
		{
			if (weapons[i]) GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Yellow, FString::Printf(TEXT("Server Weapon %d: %s"),i,*weapons[i]->GetName()));
		}
	}
	if (!HasAuthority() && GEngine) {
		for (int16 i = 0; i < weapons.Num(); i++)
		{
			if(weapons[i]) GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Yellow, FString::Printf(TEXT("Client Weapon %d: %s"), i, *weapons[i]->GetName()));
		}
	}
	
	*/
	if(GEngine && IsLocallyControlled()) GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Yellow, FString::Printf(TEXT("Move Speed : % f    Interaction Type : %s   WeaponPickup Name : %s   Current Weapon Index : %d"), GetVelocity().Length(), *UEnum::GetDisplayValueAsText(InteractionEnum).ToString(), *wepName, CurrentIndex));
	if (GEngine && IsLocallyControlled() && CurrentWeapon) GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Yellow, FString::Printf(TEXT("Current Weapon: %s Fire Mode: %s  Fire Rate: %d"),*CurrentWeapon->GetName(), *UEnum::GetDisplayValueAsText(CurrentWeapon->currentFireMode).ToString(), CurrentWeapon->fireRate));
	//if(HasAuthority() && weaponPickup) UE_LOG(LogTemp, Warning, TEXT("Pickup: %s"), *weaponPickup->weapon->GetDefaultObject()->GetName());
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Green, FString::Printf(TEXT("%s points: %f"), GetWorld()->IsServer() ? TEXT("Server ") : TEXT("Client "), localPlayerPoints));

#pragma endregion
}

void AFPSCharacter::Jump()
{
	//Dont jump if crouched or prone
	if (AdvancedMovementComponet->CurrentAdvMoveState == EAdvancedMoveState::AMS_Crouched || AdvancedMovementComponet->CurrentAdvMoveState == EAdvancedMoveState::AMS_Prone) return;
	Super::Jump();
	AdvancedMovementComponet->OnJump();
}

void AFPSCharacter::StopJumping()
{
	Super::StopJumping();
}

void AFPSCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	GetAdvancedMovement()->OnLand();
}

void AFPSCharacter::ChangeFireMode()
{
	if (CurrentWeapon) CurrentWeapon->SwitchFireMode();
}

void AFPSCharacter::ReloadWeapon()
{
	if (CurrentWeapon) CurrentWeapon->Reload();
}

void AFPSCharacter::GetValues()
{
	if(camera) defaultFOV = camera->FieldOfView;
	bCanADS = true;
	bSupressStartShot = false;
	bSupressStopShoot = false;

	if (FPSController = Cast<AFPSPlayerController>(GetController()))

	if (AdvancedMovementComponet)
	{
		AdvancedMovementComponet->bMoveCamShake = bCanCameraShake;
		AdvancedMovementComponet->OnStateChanged.AddDynamic(this, &AFPSCharacter::OnAdvStateUpdate);
	}
	else UE_LOG(LogCharacter, Error, TEXT("No Adv Movement Component Found"));
}

void AFPSCharacter::SpawnWeapon(TSubclassOf<ARangedWeapon> weapon)
{
	if (weapons.Num() == maxEquippedWeapons)
	{
		//Replace current weapon with new weapon? - potential move currentwep to tmp spot? 
		UE_LOG(LogTemp, Warning, TEXT("Replace Weapon as inventory is full"));

		FActorSpawnParameters params;
		params.Owner = this;
		ARangedWeapon* spawnedWeapon = GetWorld()->SpawnActor<ARangedWeapon>(weapon, params);
		ReplaceOwnedWeapon(spawnedWeapon, CurrentWeapon, CurrentIndex);

	}
	else
	{
		FActorSpawnParameters params;
		params.Owner = this;
		ARangedWeapon* spawnedWeapon = GetWorld()->SpawnActor<ARangedWeapon>(weapon, params);
		AddToOwnedWeapons(spawnedWeapon);
	}

	
}

void AFPSCharacter::AddToOwnedWeapons(ARangedWeapon* newWeapon)
{
	if (newWeapon == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("Add To Owned Weapons: newWeapon is null"));
		return;
	}
	else UE_LOG(LogTemp, Display, TEXT("Adding To Owned Weapons"));

	const int32 i = weapons.Add(newWeapon); //add spawned weapon and save its index to i


	
	if (i == CurrentIndex) //if new weapon is in the current equipped index
	{
		//Equip weapon
		CurrentWeapon = newWeapon; //the current weapon is your new weapon
		OnRep_CurrentWeapon(nullptr); //set the new weapon mesh to visible, nullptr because no oldmesh to turn invis 
	}
	else
	{
		//Switch to new weapon
		EquipWeapon(i);
	}
}

void AFPSCharacter::ReplaceOwnedWeapon(ARangedWeapon* newWeapon, ARangedWeapon* oldWeapon, int32 currentIndex)
{
	const int32 i = weapons.Insert(newWeapon, currentIndex);
	RemoveWeapon(oldWeapon);
	CurrentWeapon = newWeapon;
	OnRep_CurrentWeapon(nullptr);
}

void AFPSCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AFPSCharacter, weapons, COND_None);
	DOREPLIFETIME_CONDITION(AFPSCharacter, CurrentWeapon, COND_None);
	DOREPLIFETIME(AFPSCharacter, CurrentIndex);
	DOREPLIFETIME(AFPSCharacter, localPlayerPoints);
}

#pragma region Weapon Logic

void AFPSCharacter::OnRep_CurrentWeapon(const ARangedWeapon* oldWeapon)
{
	//UE_LOG(LogTemp, Display, TEXT("Rep Current Weapon"));
	if (CurrentWeapon) //Runs if currentweapon is NOT nullptr
	{
		if (!CurrentWeapon->currentOwner) //If current weapon isnt currently owned by player
		{
			const FTransform& PlacementTransform = CurrentWeapon->PlacementTransform * GetMesh()->GetSocketTransform(FName("grip_Socket_r")); //Set placement transform to move weapon to with placementTrans offset
			CurrentWeapon->SetActorTransform(PlacementTransform, false, nullptr, ETeleportType::TeleportPhysics); //Move weapon to mannaquin grip socket mesh using Placement Transform Offset
			CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepWorldTransform, FName("grip_Socket_r")); //Attach weapon to hand socket (grip_Socket_r)

			CurrentWeapon->currentOwner = this; //set current owner to player
		}
		CurrentWeapon->weaponMesh->SetVisibility(true); 
	}

	if (oldWeapon) //if there is an old weapon thats make invisible
	{
		oldWeapon->weaponMesh->SetVisibility(false);
	}
	
	
	AdvancedMovementComponet->OnRep_MoveStateUpdated();
	//UE_LOG(LogTemp, Display, TEXT("Broadcast changed weap delegate"));
	CurrentWeaponChangedDelegate.Broadcast(CurrentWeapon, oldWeapon); //works fine lmao

	//Tell HUD new weapon
	if (HUD)
	{
		HUD->currentWeaponEquipped = CurrentWeapon;
	}
}

void AFPSCharacter::EquipWeapon(const int32 index)
{
	if (!weapons.IsValidIndex(index))
	{
		UE_LOG(LogTemp, Warning, TEXT("Weapon Index Not Valid"));
		return;
	} //bail if weapon index aint real 
	if (CurrentWeapon == weapons[index])
	{
		UE_LOG(LogTemp, Warning, TEXT("Weapon Already Equipped"));
		return;
	}

	if (IsLocallyControlled() || HasAuthority())
	{
		StopShooting();

		UE_LOG(LogTemp, Display, TEXT("Equip Wepon Index: %d    "), index);
		CurrentIndex = index;

		const ARangedWeapon* oldWeapon = CurrentWeapon;
		CurrentWeapon = weapons[index];
		OnRep_CurrentWeapon(oldWeapon);
		//UE_LOG(LogTemp, Display, TEXT("Client Change Weapon"));
	}
	if (!HasAuthority()) //not local control and not server - so other clients
	{
		Server_SetCurrentWeapon(weapons[index]); //Pass to server the weapon to set
	}
}

void AFPSCharacter::StartAim()
{
	if (AdvancedMovementComponet->CurrentAdvMoveState == EAdvancedMoveState::AMS_Run)
	{
		AdvancedMovementComponet->Server_UpdateMovementState(EAdvancedMoveState::AMS_Walk);
		bCanADS = true;
	}

	if (!CurrentWeapon || !bCanADS) return;
	if (IsLocallyControlled() || HasAuthority())
	{
		bIsAiming = true;
		//Change camera FOV
		camera->SetFieldOfView(CurrentWeapon->ADSFov);
		MultiAim_Implementation(true);
		
	}
	if (!HasAuthority())
	{
		Server_Aim(true); //if client then RPC to server to run aim func that will then multicast to other clients
	}

	CalculateAimSpeed();
}

void AFPSCharacter::StopAim()
{
	if (!CurrentWeapon) return;
	if (IsLocallyControlled() || HasAuthority())
	{
		bIsAiming = false;
		camera->SetFieldOfView(defaultFOV);
		MultiAim_Implementation(false);
		AdvancedMovementComponet->OnRep_MoveStateUpdated();
	}
	if (!HasAuthority())
	{
		Server_Aim(false);
		AdvancedMovementComponet->Server_UpdateMovementState(AdvancedMovementComponet->CurrentAdvMoveState);
	}
}

void AFPSCharacter::OnAdvStateUpdate() 
{
	UE_LOG(LogCharacter, Display, TEXT("Adv State Update delegate passed to player: %s"), *GetName());

	switch (AdvancedMovementComponet->CurrentAdvMoveState)
	{
	case EAdvancedMoveState::AMS_Run:
		bCanADS = false;
		break;
	case EAdvancedMoveState::AMS_WallRun:
		bCanADS = false;
		break;
	case EAdvancedMoveState::AMS_WallClimb:
		bCanADS = false;
		break;
	case EAdvancedMoveState::AMS_WallHang:
		bCanADS = false;
		break;
	case EAdvancedMoveState::AMS_Dive:
		bCanADS = false;
		break;
	case EAdvancedMoveState::AMS_Slide:
		bCanADS = false;
		break;
	case EAdvancedMoveState::AMS_Mantle:
		bCanADS = false;
		break;
	default:
		bCanADS = true;
		break;
	}

	if (!bCanADS && bIsAiming) StopAim();

	if (bIsAiming)
	{
		CalculateAimSpeed();
	}

	//Adjust Move speed by weapon speed modifier
	if(CurrentWeapon) AdvancedMovementComponet->MaxWalkSpeed += CurrentWeapon->moveSpeedModifier;
}

void AFPSCharacter::CalculateAimSpeed()
{
	float walkSpeed;
	switch (AdvancedMovementComponet->CurrentAdvMoveState)
	{
	case EAdvancedMoveState::AMS_Walk:
		walkSpeed = walkADSSpeed;
		break;
	case EAdvancedMoveState::AMS_Crouched:
		walkSpeed = crouchedADSSpeed;
		break;
	case EAdvancedMoveState::AMS_Prone:
		walkSpeed = proneADSSpeed;
		break;
	default:
		break;
	}
	if (!HasAuthority()) Server_SetAimSpeed(walkSpeed);
	if (HasAuthority() || IsLocallyControlled()) Server_SetAimSpeed_Implementation(walkSpeed);
}

void AFPSCharacter::RemoveWeapon(ARangedWeapon* weapon)
{
	UE_LOG(LogTemp, Warning, TEXT("Removing: %s"),*CurrentWeapon->GetName());
	weapons.Remove(weapon);
	weapon->Destroy();

}

//Interaction
void AFPSCharacter::Interact()
{
	UE_LOG(LogTemp, Display, TEXT("Interact Pressed"));

	switch (InteractionEnum)
	{
	case EInteractionType::null:
		return;
		break;
	case EInteractionType::IT_MiscPickup:
		break;
	case EInteractionType::IT_Weapon:
		if (!weaponPickup) return; //Ensure weapon pickup is valid

		//Check if weapon already owned
		for (int16 i = 0; i < weapons.Num(); i++)
		{
			if (weapons[i]->GetClass()->GetDefaultObject() == weaponPickup->weapon->GetDefaultObject()) //compare default class of each weapon index with default class of weapon pickup weapon
			{
				UE_LOG(LogTemp, Warning, TEXT("Weapon Already Equipped!"));
				return;
			}
		}

		//Check cost
		if(weaponPickup->weaponCost > localPlayerPoints)
		{
			return;
		}
		else
		{
			Server_AddToLocalPlayerPoints(-weaponPickup->weaponCost);

			if (HasAuthority())
			{
				SpawnWeapon(weaponPickup->weapon);
			}
			if (!HasAuthority())
			{
				//Server spawn weapon
				Server_SpawnWeapon(weaponPickup->weapon);
			}
		}
		break;
	case EInteractionType::IT_Door:
		if(!doorInteractable || doorInteractable->DoorCost > localPlayerPoints) return;
		UE_LOG(LogTemp,Display,TEXT("Door interaction"));
		Server_TryOpenDoor(); //Ask server to open door
		Server_AddToLocalPlayerPoints(-doorInteractable->DoorCost);
		break;
	case EInteractionType::IT_Modify:
		break;
	case EInteractionType::IT_Build:
		break;
	default:
		break;
	}
}

void AFPSCharacter::InteractHold()
{
	UE_LOG(LogTemp, Display, TEXT("Interact Held"));

	switch (InteractionEnum)
	{
	case EInteractionType::null:
		break;
	case EInteractionType::IT_MiscPickup:
		break;
	case EInteractionType::IT_Weapon:
		break;
	case EInteractionType::IT_Door:
		break;
	case EInteractionType::IT_Modify:
		break;
	case EInteractionType::IT_Build:
		break;
	default:
		break;
	}
}

void AFPSCharacter::Server_SetCurrentWeapon_Implementation(ARangedWeapon* newWeapon)
{
	if (!newWeapon)
	{
		UE_LOG(LogTemp, Error, TEXT("No Weapon serverside")); return;
	}


	const ARangedWeapon* oldWeapon = CurrentWeapon;
	CurrentWeapon = newWeapon;
	UE_LOG(LogTemp, Display, TEXT("Server Change Weapon"));
	OnRep_CurrentWeapon(oldWeapon);
}

void AFPSCharacter::Server_SpawnWeapon_Implementation(TSubclassOf<ARangedWeapon> weapon)
{
	UE_LOG(LogTemp, Warning, TEXT("Server Spawn Weapon"));
	SpawnWeapon(weapon);
;}

void AFPSCharacter::NextWeapon()
{
	UE_LOG(LogCharacter, Display, TEXT("Next Weapon Input"));
	EquipWeapon(weapons.IsValidIndex(CurrentIndex + 1) ? CurrentIndex + 1 : 0); //pass either 0 (first weapon in array) or current weapon + 1 to equip weapon 
}

void AFPSCharacter::PreviousWeapon()
{
	UE_LOG(LogCharacter, Display, TEXT("Previous Weapon Input"));
	EquipWeapon(weapons.IsValidIndex(CurrentIndex - 1) ? CurrentIndex - 1 : weapons.Num() -1);	//pass current weapon - 1 or weapons length - 1(last weapon in array) to equip weapon 
}

void AFPSCharacter::MultiAim_Implementation(const bool bStarting)
{
	if (!CurrentWeapon) return;
	AimingTimeline.SetPlayRate(CurrentWeapon->ADSSpeedScalar);

	if (bStarting)
	{
		AimingTimeline.Play();
	}
	else
	{
		AimingTimeline.Reverse();
	}
}


/* TODO:
* Add SFX to play for both clients?
* Setup Bullet Pen
* Projectile Types
*/

#pragma region Shootin
void AFPSCharacter::Shoot()
{
	if (!CurrentWeapon) return;
	//UE_LOG(LogCharacter, Warning, TEXT("Shooting"));
	bIsShooting = true;

	switch (CurrentWeapon->projectileType)
	{
	case EProjectileType::PT_HitScan:

		switch (CurrentWeapon->currentFireMode)
		{
		case EFireMode::FM_SingleFire:
			if (!bSupressStartShot)
			{
				ShootHitscan();
				GetWorld()->GetTimerManager().SetTimer(betweenShotsTimer, this, &AFPSCharacter::ResetShooting, CurrentWeapon->timebetweenSingleShots, false);
				bSupressStartShot = true;
			}
			break;
		case EFireMode::FM_BurstFire:
			if (!bSupressStartShot)
			{
				burstCounter = CurrentWeapon->burstCount;
				bSupressStopShoot = true;
				bSupressStartShot = true;
				GetWorld()->GetTimerManager().SetTimer(shotTimer, this, &AFPSCharacter::ShootHitscan, 1 / float(CurrentWeapon->burstFireRate), true);
				GetWorld()->GetTimerManager().SetTimer(betweenShotsTimer, this, &AFPSCharacter::ResetShooting, CurrentWeapon->timebetweenSingleShots, false);
				
			}
			break;
		case EFireMode::FM_FullAutoFire:
			if (!bSupressStartShot)
			{
				//UE_LOG(LogCharacter, Warning, TEXT("Fire Full Auto"));
				bSupressStartShot = true;
				GetWorld()->GetTimerManager().SetTimer(shotTimer, this, &AFPSCharacter::ShootHitscan, 1/float(CurrentWeapon->fireRate), true,0.0f);
				GetWorld()->GetTimerManager().SetTimer(betweenShotsTimer, this, &AFPSCharacter::ResetShooting, CurrentWeapon->timebetweenSingleShots, false);
			}

			break;
		default:
			break;
		}

		break;
		/*
	case EProjectileType::PT_Projectile:
		break;
	case EProjectileType::PT_Custom:
		break;
		*/
	default:
		break;
	}

}

void AFPSCharacter::StopShooting()
{
	if (!bIsShooting || !CurrentWeapon || bSupressStopShoot) return;

	GetWorld()->GetTimerManager().ClearTimer(shotTimer);
	bIsShooting = false;
}

void AFPSCharacter::ResetShooting()
{
	GetWorld()->GetTimerManager().ClearTimer(betweenShotsTimer);
	bSupressStartShot = false;
}

//TODO: ADD VALVE HIT COMPENSATION?
void AFPSCharacter::ShootHitscan()
{
	if (CurrentWeapon->UpdateAmmo())
	{
	
		if (!CurrentWeapon->rangeCurve) { UE_LOG(LogTemp, Error, TEXT("No Damage Curve Assigned to Current Weapon")); return; }
		float MaxRange; float min;
		CurrentWeapon->rangeCurve->GetTimeRange(min, MaxRange);

		if (CurrentWeapon->currentFireMode == EFireMode::FM_BurstFire)
		{
			if (--burstCounter <= 0)
			{
				bSupressStopShoot = false;
				StopShooting();
			}
		}


		if (!HasAuthority() || IsLocallyControlled()) //if local player or not server
		{
			FHitResult hit;
			FCollisionQueryParams params;
			params.AddIgnoredActor(this);

			if (GetWorld()->LineTraceSingleByChannel(hit, camera->GetComponentLocation(), (camera->GetForwardVector() * MaxRange) + camera->GetComponentLocation(), ECC_Visibility, params)) {
				
				//Debug Hit Point
				DrawDebugSphere(GetWorld(), hit.ImpactPoint, 5.f, 12, FColor::Purple, false, 4.f);

				//Change point with wep accuracy
				//TODO: Make weapon innaccuracy not punish range?

				//const FVector shotPoint = hit.ImpactPoint + UKismetMathLibrary::RandomUnitVectorInConeInDegrees(hit.ImpactNormal, CurrentWeapon->weaponAccuracy/2) * UKismetMathLibrary::RandomFloatInRange(0, CurrentWeapon->weaponAccuracy);
				const FVector shotPoint = (hit.ImpactPoint + (hit.ImpactNormal * 2.f) + UKismetMathLibrary::RandomUnitVector() * UKismetMathLibrary::RandomFloatInRange(0, CurrentWeapon->weaponAccuracy));
				
				
				//Add small extra range for deviation to ensure always hit

				Server_GetHitscanHit(CurrentWeapon->GetMuzzleWorldTransform().GetLocation(), shotPoint, MaxRange);
			}
			else
			{
				UE_LOG(LogWeapons, Warning, TEXT("FallBack Ray Cast Used"));
				Server_GetHitscanHit(CurrentWeapon->GetMuzzleWorldTransform().GetLocation(),camera->GetForwardVector() * MaxRange + CurrentWeapon->GetMuzzleWorldTransform().GetLocation() + CurrentWeapon->PlacementTransform.GetLocation(), MaxRange);
			}
			

			
			/*
			if (bIsAiming)
			{
				Server_GetHitscanHit(CurrentWeapon->GetSightsWorldTransform().GetLocation(), (camera->GetForwardVector() * MaxRange) + CurrentWeapon->GetSightsWorldTransform().GetLocation(), MaxRange);
			}
			else
			{
				Server_GetHitscanHit(camera->GetComponentLocation(), (camera->GetForwardVector() * MaxRange) + camera->GetComponentLocation(), MaxRange);
			}
			*/
			//FX TODO -> Check acc plays sound for both players
			CurrentWeapon->Server_PlayFX();
			//UE_LOG(LogCharacter, Display, TEXT("Shot Fired Client/Local Control"));
		}

	}
	else
	{
		bSupressStopShoot = false;
		StopShooting();
		if (bAutoReload)
		{
			ReloadWeapon();
		}
	}
}


void AFPSCharacter::Server_GetHitscanHit_Implementation(const FVector startPos, const FVector endPos, const float weaponMaxRange)
{
	FHitResult hit;
	FCollisionQueryParams params;
	params.AddIgnoredActor(this);
	

	//Do Hit Scan
	if (GetWorld()->LineTraceSingleByChannel(hit, startPos, endPos, ECC_GameTraceChannel2, params))
	{
		DrawDebugLine(GetWorld(), startPos, hit.ImpactPoint, FColor::Magenta, false, 3.5f);	
		DrawDebugSphere(GetWorld(), hit.ImpactPoint, 8.f, 12, FColor::Red, false, 4.f);
		//If hit registered

		//If hit another player (friendly fire)
		if (hit.GetActor()->ActorHasTag("Player"))
		{
			UE_LOG(LogCharacter, Display, TEXT("Hit Player!"));
			return;
		}
		else if (TObjectPtr<UHealthComponent> targetHealth = Cast<UHealthComponent>(hit.GetActor()->GetComponentByClass<UHealthComponent>()))
		{
			targetHealth->TakeDamage(CurrentWeapon->damagePerBullet);
			localPlayerPoints += CurrentWeapon->pointPerHit;
		}

		Client_OnHit(hit);
	}
}


void AFPSCharacter::Client_OnHit_Implementation(const FHitResult actorHit)
{
	return;
}

#pragma endregion


#pragma endregion

void AFPSCharacter::Sprint()
{
	GetAdvancedMovement()->Sprint();
}

void AFPSCharacter::DoCrouch()
{
	GetAdvancedMovement()->CheckCrouch();
}

void AFPSCharacter::Prone()
{
	GetAdvancedMovement()->Prone();
}

void AFPSCharacter::TimelineProgress(const float value)
{
	adsWeight = value;
	fovWeight = value;
}

void AFPSCharacter::Server_TryOpenDoor_Implementation()
{
	UE_LOG(LogTemp,Warning,TEXT("Door open attempt on server?"));
	doorInteractable->OpenDoor_Implementation();
}

#pragma region Camera
void AFPSCharacter::CameraShake(const TSubclassOf<UCameraShakeBase> camShake, const float shakeScale)
{
	if (!IsLocallyControlled() || !HasActiveCameraComponent() || !camShake || !bCanCameraShake) return;

	if (GetWorld()->GetFirstLocalPlayerFromController())
	{
		GetWorld()->GetFirstLocalPlayerFromController()->PlayerController->ClientStartCameraShake(camShake, shakeScale);
	}
	else { UE_LOG(LogTemp, Error, TEXT("No Player Controller found!")); return; }
}
#pragma endregion

