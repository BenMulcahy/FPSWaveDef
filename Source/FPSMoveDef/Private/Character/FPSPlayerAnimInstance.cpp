// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/FPSPlayerAnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Curves/CurveVector.h"
#include "GameFramework/PawnMovementComponent.h"
#include "..\..\Public\Character\FPSCharacter.h"

UFPSPlayerAnimInstance::UFPSPlayerAnimInstance()
{
    
}

void UFPSPlayerAnimInstance::NativeBeginPlay()
{
    Super::NativeBeginPlay();
}

void UFPSPlayerAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
    Super::NativeUpdateAnimation(DeltaTime);
    
    if (!character)
    {
        character = Cast<AFPSCharacter>(TryGetPawnOwner());

        if (character)
        {
            mesh = character->GetMesh();

            character->CurrentWeaponChangedDelegate.AddDynamic(this, &UFPSPlayerAnimInstance::CurrentWeaponChanged);
            CurrentWeaponChanged(character->CurrentWeapon, nullptr);

            //UE_LOG(LogTemp, Warning, TEXT("Character found - Mesh: %s"), *mesh->GetName());
        }
        else return;
    }

    SetVars(DeltaTime);
    CalculateWeaponSway(DeltaTime);

    LastRotation = camTransform.Rotator();

}

void UFPSPlayerAnimInstance::SetVars(const float DeltaTime)
{
    camTransform = FTransform(character->GetBaseAimRotation(), character->camera->GetComponentLocation()); //using control rotaion cus cam rotation isnt replicated by default but aim rot is driving cam rot anyway
    
    //rel cam trans
    const FTransform& rootOffset = mesh->GetSocketTransform(FName("root"), RTS_Component).Inverse() * mesh->GetSocketTransform(FName("ik_hand_root")); //not full needed but if mesh has built in rotation to the pose this will ensure rel cam pos is calc'd correctly - uses UE5 base skel tho
    relCamTransform = camTransform.GetRelativeTransform(rootOffset);

 
#pragma region WeaponSway Values

    constexpr float AngleClamp = 6.f; //prevent sway from going too far
    FRotator AddRotation = FRotator(FMath::ClampAngle((camTransform.Rotator() - LastRotation).Pitch, -AngleClamp, AngleClamp) * 1.5f, FMath::ClampAngle((camTransform.Rotator() - LastRotation).Yaw, -AngleClamp, AngleClamp), 0.f);
    AddRotation.Roll = AddRotation.Yaw * 0.7f;

    AccumulativeRotation += AddRotation;
    AccumulativeRotation = UKismetMathLibrary::RInterpTo(AccumulativeRotation, FRotator::ZeroRotator, DeltaTime, swayToZeroSpeed); //Reverse interp back to 0
    AccumulativeRotInterp = UKismetMathLibrary::RInterpTo(AccumulativeRotInterp, AccumulativeRotation, DeltaTime, swaySpeed);

    float playerVelocity = character->GetVelocity().Length();
    float playerMaxSpeed = character->AdvancedMovementComponet->GetAbsoluteMaxPlayerSpeed();
    //UE_LOG(LogTemp, Warning, TEXT("Player abs Max: %f"), playerMaxSpeed);

    if (character->defaultWepBob)
    {
        const UCurveVector* curve = character->defaultWepBob;
        float tmpVel = UKismetMathLibrary::NormalizeToRange(playerVelocity, (playerMaxSpeed * 0.8f) * -1, playerMaxSpeed);
        FVector AddLocation = FVector(curve->GetVectorValue(playerVelocity >= 10.f ? GetWorld()->GetTimeSeconds() * 2.0 : GetWorld()->GetTimeSeconds() * 0.25));
        if (character->GetMovementComponent()->IsFalling()) {
            AddLocation *= 0.1f; //Scaling sway when falling
        } 
        AccumulativeLocation = UKismetMathLibrary::VInterpTo(AccumulativeLocation, AddLocation, DeltaTime, weaponBopSpeed);
        AccumulativeLocationInterp = UKismetMathLibrary::VInterpTo(AccumulativeLocationInterp, AccumulativeLocation, DeltaTime, weaponBopSpeed);
        AccumulativeLocationInterp *= tmpVel;
        AccumulativeLocationInterp.X = 0.f;
        //UE_LOG(LogTemp, Warning, TEXT("tmpVel: %f"), tmpVel);
    }


#pragma endregion

    if (character) {
        adsWeight = character->adsWeight;
        proneWeight = character->proneWeight;
    }
}

void UFPSPlayerAnimInstance::CalculateWeaponSway(const float DeltaTime)
{
    FVector locationOffset = FVector::ZeroVector;
    FRotator rotationOffset = FRotator::ZeroRotator;

    //////////////////////////////////////////////////Aim Sway
    const FRotator& AccumulativeRotInterpInverse = AccumulativeRotInterp.GetInverse();
    rotationOffset += AccumulativeRotInterpInverse;
    locationOffset += FVector(0.f, AccumulativeRotInterpInverse.Yaw, AccumulativeRotInterpInverse.Pitch) / 6.f;
    locationOffset += AccumulativeLocationInterp;
    
    if (character->bIsAiming)
    {
        rotationOffset.Pitch *= ikProperties.adsWeaponSwayScale;
        rotationOffset.Yaw *= ikProperties.adsWeaponSwayScale;
        rotationOffset.Roll *= ikProperties.adsWeaponSwayScale;
        locationOffset *= ikProperties.adsWeaponSwayScale;
    }
    else if(!character->bIsAiming)
    {
        rotationOffset.Pitch *= ikProperties.weaponSwayScale;
        rotationOffset.Yaw *= ikProperties.weaponSwayScale;
        rotationOffset.Roll *= ikProperties.weaponSwayScale;
        locationOffset *= ikProperties.weaponSwayScale;
    }

    ////////////////////////////////////////////////Weap Sway Idle
    
    if (character->bIsAiming)
    {
        locationOffset.Z *= ikProperties.adsWeaponBobScale.Z;
        locationOffset.Y *= ikProperties.adsWeaponBobScale.Y;
    }
    else
    {
        locationOffset.Z *= ikProperties.weaponBobScale.Z;
        locationOffset.Y *= ikProperties.weaponBobScale.Y;
    }
    

    swayOffsetTransform = FTransform(rotationOffset, locationOffset);
 
}

void UFPSPlayerAnimInstance::SetIKTransforms()
{
    rHandToSightsTrans = currentWeapon->GetSightsWorldTransform().GetRelativeTransform(mesh->GetSocketTransform(FName("hand_r")));
    //UE_LOG(LogTemp, Error, TEXT("IK Trans set"));
}

void UFPSPlayerAnimInstance::CurrentWeaponChanged(ARangedWeapon* newWep, const ARangedWeapon* oldWep)
{
    //UE_LOG(LogTemp, Error, TEXT("Anim weapon changed"));
    currentWeapon = newWep;
    if (currentWeapon)
    {
        ikProperties = currentWeapon->ikProperties;
        GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UFPSPlayerAnimInstance::SetIKTransforms); //wait a tick before setting ik values, prevents ik values being set before anim pose updates for new weapon
    }
}

