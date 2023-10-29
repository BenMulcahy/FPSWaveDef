// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/RangedWeapon.h"
#include "NiagaraFunctionLibrary.h"
#include "FPSMoveDef/FPSMoveDef.h"
#include "Kismet/KismetMathLibrary.h"

//TODO: Sound not playing on all clients?

ARangedWeapon::ARangedWeapon()
{
    PrimaryActorTick.bCanEverTick = false; //Prevent void tick
    bReplicates = true;

    root = CreateDefaultSubobject<USceneComponent>(TEXT("root"));
    RootComponent = root;

    weaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon Mesh"));
    weaponMesh->SetupAttachment(root);
    weaponMesh->bCastDynamicShadow = false;

    weaponAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("Weapon Audio Component"));
    weaponAudioComponent->bAutoActivate = false;
    weaponAudioComponent->SetupAttachment(weaponMesh,"Muzzle");
}

void ARangedWeapon::BeginPlay()
{
    Super::BeginPlay();
    
    if(!currentOwner) weaponMesh->SetVisibility(false);
    if (shotSFX && weaponAudioComponent)
    {
        weaponAudioComponent->SetSound(shotSFX);
    }
    currentFireMode = defaultFireMode;
    currentBulletCount = magSize;
    currentReserveCount = maxReserveAmmo;
}

void ARangedWeapon::Tick(const float DeltaTime)
{
    //if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Magenta, FString::Printf(TEXT("%s WepNiagSys: %s"), *this->GetName(), weaponNiagaraComponent? TEXT("Exists"):TEXT("Null")));
    //if (GEngine && GetWorld()->IsNetMode(ENetMode::NM_Client) && HasLocalNetOwner()) GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Magenta, FString::Printf(TEXT("Current Bullet Count: %d   Reserve Ammo: %d"), currentBulletCount, currentReserveCount));
}

void ARangedWeapon::SwitchFireMode()
{
    if (fireModes.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("No FireModes Set!"));
        return;
    }
    else
    {
        int32 i;
        for (int32 x = 0; x < fireModes.Num(); x++)
        {
            if (currentFireMode == fireModes[x]) i = x;
        }
        currentFireMode = fireModes[fireModes.IsValidIndex(i + 1) ? i + 1 : 0];
    }
}

void ARangedWeapon::PlayMuzzleFlash()
{
    if (!muzzleFlash) { UE_LOG(LogTemp, Error, TEXT("No Muzzle Flash Assigned!")); return; }
    
    if(!weaponNiagaraComponent)
    {
        weaponNiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(muzzleFlash, weaponMesh, "Muzzle", FVector(0.f), FRotator(-90.f, 0.f, -90.f), EAttachLocation::Type::KeepRelativeOffset, false, false);
        weaponNiagaraComponent->ActivateSystem();
    }
    else
    {
        weaponNiagaraComponent->ActivateSystem();
    }
}

void ARangedWeapon::MultiPlayWeaponFX_Implementation()
{
    if (GetWorld()->IsNetMode(ENetMode::NM_Client) || GetWorld()->IsNetMode(ENetMode::NM_Standalone) || GetWorld()->IsNetMode(ENetMode::NM_ListenServer))
    {
        PlayShotSound();
        PlayMuzzleFlash();
    }
}

void ARangedWeapon::PlayShotSound()
{
    if (shotSFX && weaponAudioComponent)
    {
        if (weaponAudioComponent->Sound)
        {
            //UE_LOG(LogWeapons, Warning, TEXT("Player: %s played sound at location %s"), *GetOwner()->GetName(), *weaponAudioComponent->GetComponentLocation().ToString());
            weaponAudioComponent->SetFloatParameter(FName("pitch"), UKismetMathLibrary::RandomFloatInRange(0.9f, 1.1f));
            weaponAudioComponent->Play();
        }
    }
    else UE_LOG(LogTemp, Warning, TEXT("No Sound FX Assigned!"));
}

bool ARangedWeapon::UpdateAmmo()
{
    if (currentBulletCount > 0)
    {
        --currentBulletCount;
        return true;
    }
    else return false;
}

void ARangedWeapon::Reload()
{
    if (bIsReloading) return;

    if (currentReserveCount <= 0)
    {
        UE_LOG(LogWeapons, Display, TEXT("Out of Ammo!"));
        return;
    }
    else
    {
        bIsReloading = true;
        UE_LOG(LogWeapons, Display, TEXT("Reloading..."));
        GetWorld()->GetTimerManager().SetTimer(reloadTimer, this, &ARangedWeapon::DoReload, timeToReload, false);
    }
}

void ARangedWeapon::DoReload()
{
    int32 ammoNeeded = magSize - currentBulletCount;

    if (ammoNeeded > currentReserveCount + currentBulletCount)
    {
        //more ammo required than currently owned - set current bullet count to be all reserve up to mag size
        currentBulletCount = UKismetMathLibrary::Clamp(currentReserveCount + currentBulletCount, 0, magSize);
        currentReserveCount = 0;
    }
    else
    {
        currentReserveCount = UKismetMathLibrary::Clamp(currentReserveCount -= ammoNeeded, 0, maxReserveAmmo);
        currentBulletCount = magSize;
    }
    bIsReloading = false;
}
