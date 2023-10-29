// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AdvMovementComponent.h"
#include "GameFramework/Character.h"
#include "FPSMoveDef/FPSMoveDef.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"

//TODO:
// Ledge Hang?
// Add Dolphin Dive
// Adjust Wall Run to only use gravity for the latter half of the wall run + cancel vert moment on start of wall run
// Mantle Tag?

UAdvMovementComponent::UAdvMovementComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    SetIsReplicatedByDefault(true);
}

void UAdvMovementComponent::BeginPlay()
{
    Super::BeginPlay();
    
    SetValues();

    //Start PK checks
    GetWorld()->GetTimerManager().SetTimer(parkourCheckTimer, this, &UAdvMovementComponent::ParkourTick, 0.0167f, true);
}

void UAdvMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    CheckSprint();

   
    //Debug:
    /*
    if (GEngine && GetCharacterOwner()->IsLocallyControlled()) GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Cyan, FString::Printf(TEXT("AdvMvmnt - %s: CState: %s  PState: %s"), GetWorld()->IsServer() ? *GetWorld()->GetName() : *GetOwner()->GetName(), *UEnum::GetDisplayValueAsText(CurrentAdvMoveState).ToString(), *UEnum::GetDisplayValueAsText(PreviousAdvMoveState).ToString()));
    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Magenta, FString::Printf(TEXT("Wall Run: bCanWallRun: %s  bIsAbleToWallRun: %s, Wall Run State: %s"), bCanWallRun ? TEXT("True") : TEXT("False"), bIsAbleToWallRun ? TEXT("True") : TEXT("False"),*UEnum::GetDisplayValueAsText(WallRunState).ToString()));
    */
}

void UAdvMovementComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UAdvMovementComponent, CurrentAdvMoveState);
    DOREPLIFETIME(UAdvMovementComponent, bIsAbleToWallRun);
    DOREPLIFETIME(UAdvMovementComponent, PreviousAdvMoveState);
    DOREPLIFETIME(UAdvMovementComponent, WallRunState);
    DOREPLIFETIME(UAdvMovementComponent, bIsAbleToWallClimb);
    DOREPLIFETIME(UAdvMovementComponent, bWantsToMantle);

}

void UAdvMovementComponent::SetValues()
{
    defaultGravity = GravityScale;
    wallRunMaxSpeed = sprintMoveSpeed + wallRunMoveSpeedModifier;
    wallRunJumpHeight = JumpZVelocity + wallRunJumpOffHeightModifier;
    wallClimbJumpHeight = JumpZVelocity + wallClimbJumpOffHeightModifier;
    slideSpeed = MaxWalkSpeedCrouched + slideSpeedModifier;
    defaultWalkSpeed = MaxWalkSpeed;
    UE_LOG(LogAdvMovement, Display, TEXT("Advanced Movement Values Set"));
    bIsAbleToWallRun = true;
    //bIsAbleToMantle = true;
    bIsAbleToWallClimb = true;
    bValuesSet = true;
}

void UAdvMovementComponent::OnLand()
{
    //UE_LOG(LogAdvMovement, Display, TEXT("Landed"));
    CameraShake(JumpLandShake, 1.f);

    if (!bIsAbleToWallRun)
    {
        WallRunEnd();
        ResetWallRun();

    }
    if (!bIsAbleToWallClimb)
    {
        WallClimbEnd();
        ResetWallClimb();
    }

    if (bWantsToMantle)
    {
        bWantsToMantle = false;
        WallClimbEnd();
        ResetWallClimb();
    }

}

#pragma region MoveStates

void UAdvMovementComponent::Server_UpdateMovementState_Implementation(const EAdvancedMoveState ToState)
{
    UE_LOG(LogTemp, Display, TEXT("Update Move State"));
    //Save Previous Move State if updated state is different to current state
    if(ToState != CurrentAdvMoveState) PreviousAdvMoveState = CurrentAdvMoveState;
    //Update new move state
    CurrentAdvMoveState = ToState;
    OnRep_MoveStateUpdated();
}

//Called on both client and server (server calls in update move state, movestate rep calls on client)
void UAdvMovementComponent::OnRep_MoveStateUpdated()
{
    if (!GetWorld()->IsServer()) { UE_LOG(LogAdvMovement, Display, TEXT("Client MoveStateUpdated Logic")); }
    else UE_LOG(LogAdvMovement, Display, TEXT("Server MoveStateUpdate Logic"));

    
    //Any Logic When Switching State
    switch (CurrentAdvMoveState)
    {
    case EAdvancedMoveState::AMS_Walk:
        MaxWalkSpeed = defaultWalkSpeed;
        break;
    case EAdvancedMoveState::AMS_Run:
        MaxWalkSpeed = sprintMoveSpeed;
        break;
    case EAdvancedMoveState::AMS_Crouched:
        MaxWalkSpeed = MaxWalkSpeedCrouched;
        break;
    case EAdvancedMoveState::AMS_Prone:
        MaxWalkSpeed = proneMoveSpeed;
        break;
    case EAdvancedMoveState::AMS_WallRun:
        break;
    case EAdvancedMoveState::AMS_WallClimb:
        break;
    case EAdvancedMoveState::AMS_WallHang:
        break;
    case EAdvancedMoveState::AMS_Dive:
        break;
    case EAdvancedMoveState::AMS_Slide:
        StartSlide();
        break;
    case EAdvancedMoveState::AMS_Mantle:

        break;
    default:
        break;
    }

    //Broadcast MoveState Changed
    OnStateChanged.Broadcast();
}

#pragma endregion

#pragma region Base Movement
#pragma region Sprinting
void UAdvMovementComponent::Sprint()
{
    if (!CheckSprint()) return;

    if (CurrentAdvMoveState != EAdvancedMoveState::AMS_Run)
    {
        if (CurrentAdvMoveState == EAdvancedMoveState::AMS_Prone) StopProne();
        if (CurrentAdvMoveState == EAdvancedMoveState::AMS_Crouched) StopCrouch();
        StartSprint();
    }
    else
    {
        StopSprint();
    }
}

bool UAdvMovementComponent::CheckSprint()
{
    if (bCanSprint)
    {
        //if current fwdinput is not greater than 0
        if (GetForwardInput() <= 0.f)
        {
            if(CurrentAdvMoveState == EAdvancedMoveState::AMS_Run) StopSprint(); //if currently running then stop running
            return false;
        }
        return true; //if can sprint and none other checks returned
    }
    else return false; //if cant sprint
}



void UAdvMovementComponent::StartSprint()
{
    if (!GetCharacterOwner()->HasAuthority() && GetCharacterOwner()->IsLocallyControlled())
    {
        Server_UpdateMovementState(EAdvancedMoveState::AMS_Run);
    }
}

void UAdvMovementComponent::StopSprint()
{
    if (!GetCharacterOwner()->HasAuthority() && GetCharacterOwner()->IsLocallyControlled())
    {
        Server_UpdateMovementState(EAdvancedMoveState::AMS_Walk);
    }
}

#pragma endregion

#pragma region Crouching

void UAdvMovementComponent::CheckCrouch()
{
    if (CurrentAdvMoveState == EAdvancedMoveState::AMS_Run)
    {
        Slide();
    }
    else
    {
        DoCrouch();
    }
}

void UAdvMovementComponent::DoCrouch()
{
    if (CurrentAdvMoveState != EAdvancedMoveState::AMS_Crouched)
    {
        StartCrouch();
    }
    else
    {
        StopCrouch();
    }
}

void UAdvMovementComponent::StartCrouch()
{

    if (!GetCharacterOwner()->HasAuthority() && GetCharacterOwner()->IsLocallyControlled())
    {
        Server_UpdateMovementState(EAdvancedMoveState::AMS_Crouched);
    }
}

void UAdvMovementComponent::StopCrouch()
{
    if (!GetCharacterOwner()->HasAuthority() && GetCharacterOwner()->IsLocallyControlled())
    {
        Server_UpdateMovementState(EAdvancedMoveState::AMS_Walk);
    }
}

#pragma endregion

#pragma region Sliding
void UAdvMovementComponent::Slide()
{
    if (CurrentAdvMoveState != EAdvancedMoveState::AMS_Slide)
    {
        //Set Move State to Sliding
        Server_UpdateMovementState(EAdvancedMoveState::AMS_Slide);
    }
}


void UAdvMovementComponent::StartSlide()
{
    UE_LOG(LogAdvMovement, Warning, TEXT("Sliding!"));
    //Calculate Slide Direction
    slideVector = GetCharacterOwner()->GetActorForwardVector() * slideSpeed;

    GetWorld()->GetTimerManager().SetTimer(slideDurationTimer, this, &UAdvMovementComponent::StopSlide, slideDuration, false);
}



void UAdvMovementComponent::StopSlide()
{
    UE_LOG(LogAdvMovement, Warning, TEXT("Stop Slide"));
    Server_UpdateMovementState(EAdvancedMoveState::AMS_Crouched);
}
#pragma endregion

#pragma region Prone
void UAdvMovementComponent::Prone()
{
    if (CurrentAdvMoveState != EAdvancedMoveState::AMS_Prone)
    {
        StartProne();
    }
    else
    {
        StopProne();
    }
}

void UAdvMovementComponent::StartProne()
{
    if (!GetCharacterOwner()->HasAuthority() && GetCharacterOwner()->IsLocallyControlled())
    {
        Server_UpdateMovementState(EAdvancedMoveState::AMS_Prone);
    }
}

void UAdvMovementComponent::StopProne()
{
    if (!GetCharacterOwner()->HasAuthority() && GetCharacterOwner()->IsLocallyControlled())
    {
        if (PreviousAdvMoveState == EAdvancedMoveState::AMS_Run) Server_UpdateMovementState(EAdvancedMoveState::AMS_Walk);
        Server_UpdateMovementState(PreviousAdvMoveState);
    }
}
#pragma endregion

//TODO: Refactor? Dont like the OnJump thing and calling Super::Jump in character etc
void UAdvMovementComponent::OnJump()
{
    //Wall Jump
    if (CurrentAdvMoveState == EAdvancedMoveState::AMS_WallRun)
    {
        Server_WallJump();
        Server_WallJump_Implementation();
    }

    //Ledge Jump
    if (CurrentAdvMoveState == EAdvancedMoveState::AMS_WallClimb || CurrentAdvMoveState == EAdvancedMoveState::AMS_WallHang || CurrentAdvMoveState == EAdvancedMoveState::AMS_Mantle)
    {
        WallClimbEnd();
        Server_LaunchCharacter(FVector(wallClimbImpactNormal.X * wallJumpOffForce,wallClimbImpactNormal.Y * wallJumpOffForce,wallClimbJumpHeight), false, true, true);
        Server_LaunchCharacter_Implementation(FVector(wallClimbImpactNormal.X * wallJumpOffForce, wallClimbImpactNormal.Y * wallJumpOffForce, wallClimbJumpHeight), false, true, true);
    }

}

void UAdvMovementComponent::JumpFromWall()
{
    GetCharacterOwner()->LaunchCharacter(FVector(wallJumpOffForce * wallRunNormal.X, wallJumpOffForce * wallRunNormal.Y, wallRunJumpOffHeightModifier), false, true);
    ForceResetJumps();
}

#pragma endregion

#pragma region Parkour
void UAdvMovementComponent::ParkourTick()
{
    //Return if currently cannot parkour D:
    if (!bCanParkour) return;

    //Check For Wall Run
    if (bCanWallRun && bIsAbleToWallRun) WallRunTick();
   

    //Check for Wall Climb
    if (bCanWallClimb && bIsAbleToWallClimb) WallClimbTick(); //Wall Climb check

    if (bCanMantle && bWantsToMantle) CheckMantleTick();
}

#pragma region Wall Running

void UAdvMovementComponent::WallRunTick()
{   
    //Check Right Wall
    if (CheckWallRun(GetOwner()->GetActorLocation(), CalculateWallRunVector(false), -1))
    {
        //if wall running right
        if (WallRunState == EAdvWallRunState::WR_null)
        {
            UE_LOG(LogAdvMovement, Display, TEXT("Wall Run Right"));
            WallRunState = EAdvWallRunState::WR_R;
        }

        return;
    }

    //Check Left Wall
    if (CheckWallRun(GetOwner()->GetActorLocation(), CalculateWallRunVector(true), 1))
    {
        //if wall running Left
        if (WallRunState == EAdvWallRunState::WR_null)
        {
            UE_LOG(LogAdvMovement, Display, TEXT("Wall Run Left"));
            WallRunState = EAdvWallRunState::WR_L;
        }


        return;
    }

    if (WallRunState != EAdvWallRunState::WR_null)
    {
        //Only run if not running left or right
        WallRunEnd();
    }

}

bool UAdvMovementComponent::CheckWallRun(const FVector lineTraceStartLocation, const FVector lineTraceEndLocation, float wallRunDir)
{
    if (!IsFalling()) return false; //Dont Wall Run if not falling

    //Line Trace Parameters
    FHitResult hit;
    FCollisionQueryParams params;
    params.AddIgnoredActor(GetOwner());

    
    if (GetWorld()->LineTraceSingleByChannel(hit, lineTraceStartLocation, lineTraceEndLocation, ECC_GameTraceChannel3, params))
    {
        if (!hit.GetActor()) return false;

        if (hit.bBlockingHit)DrawDebugLine(GetWorld(), lineTraceStartLocation, hit.Location, FColor::Red, false, 10.f);
        //DrawDebugLine(GetWorld(), lineTraceStartLocation, lineTraceEndLocation, FColor::Green, false, 10.f);

        if (hit.bBlockingHit && UKismetMathLibrary::InRange_FloatFloat(hit.Normal.Z, -0.52, 0.52) && hit.GetActor()->ActorHasTag(wallRunTag))
        {
            wallRunNormal = hit.Normal;
            FVector vectorToWall = wallRunNormal - GetCharacterOwner()->GetActorLocation().Length();
            const FVector vectorFwd = wallRunNormal.Cross(FVector::UpVector) * (wallRunMaxSpeed * wallRunDir);
            vectorToWall *= wallRunNormal;

            DoWallRun(vectorFwd, vectorToWall);
            return true;
        }

    }
    return false; 
}

void UAdvMovementComponent::DoWallRun(const FVector fwd, const FVector wall)
{
    if(CurrentAdvMoveState != EAdvancedMoveState::AMS_WallRun) Server_UpdateMovementState(EAdvancedMoveState::AMS_WallRun);

    GravityScale = UKismetMathLibrary::FInterpTo(GravityScale, WallRunGravity, GetWorld()->DeltaTimeSeconds, 10.f);
    
    //Set timer for wall run duration should be called first frame of wall run
    if (GetWorld()->GetTimerManager().GetTimerRemaining(wallRunDurationTimer) <= 0.f)
    {
        Server_LaunchCharacter(wall, false, false);
        Server_LaunchCharacter_Implementation(wall, false, false);
        //GetCharacterOwner()->LaunchCharacter(wall, false, true);
        GetWorld()->GetTimerManager().SetTimer(wallRunDurationTimer, this, &UAdvMovementComponent::WallRunEnd, 1.0f, false, wallRunDuration);
    }
    
    //GetCharacterOwner()->LaunchCharacter(fwd, true, !bWallRunGravity);
    Server_LaunchCharacter(fwd, true, !bWallRunGravity);
    Server_LaunchCharacter_Implementation(fwd, true, !bWallRunGravity);
}

void UAdvMovementComponent::WallRunEnd()
{
    UE_LOG(LogAdvMovement, Warning, TEXT("Wall Run End"));
    GravityScale = defaultGravity;
    WallRunState = EAdvWallRunState::WR_null;
    Server_UpdateMovementState(EAdvancedMoveState::AMS_Run);
    bIsAbleToWallRun = false;
    GetWorld()->GetTimerManager().ClearTimer(wallRunDurationTimer);
    GetWorld()->GetTimerManager().SetTimer(wallRunSupressedTimer, this, &UAdvMovementComponent::ResetWallRun, 1.0f, false , timeBetweenWallRuns);
}

void UAdvMovementComponent::ResetWallRun()
{
    UE_LOG(LogAdvMovement, Display, TEXT("wall run reset"));
    GetWorld()->GetTimerManager().ClearTimer(wallRunSupressedTimer);
    bIsAbleToWallRun = true;
}

FVector UAdvMovementComponent::CalculateWallRunVector(const bool bIsLeft)
{
    if (!GetCharacterOwner()) {
        UE_LOG(LogAdvMovement, Error, TEXT("CalculateWallRunEndPoint(): No Character Assigned to Additional Movement Component"));
        return FVector(NULL);
    }
    const FVector right = GetCharacterOwner()->GetActorRightVector() * 75.f;
    const FVector fwd = GetCharacterOwner()->GetActorForwardVector() * -35.f;
    const FVector left = GetCharacterOwner()->GetActorRightVector() * -75.f;

    if (bIsLeft)return FVector(GetCharacterOwner()->GetActorLocation() + left + fwd);
    else return FVector(GetCharacterOwner()->GetActorLocation() + right + fwd);
}

#pragma endregion

#pragma region Wall Climbing

void UAdvMovementComponent::WallClimbTick()
{
    //Check if should wall climb
    if (CanWallClimb())
    {
        //Cast from eyes to feet in front of player to check for mantle surface
        if (WallClimbSweepCast(CalculateWallClimbVector(true), CalculateWallClimbVector(false)))
        {
            UE_LOG(LogAdvMovement, Warning, TEXT("I Wanna mantle"));
            //wants to wall hang/mantle
            bIsAbleToWallClimb = false;
            bWantsToMantle = true;
            Server_UpdateMovementState(EAdvancedMoveState::AMS_Mantle);
        }
        else
        {
            //Do wall climb
            DoWallClimb();
        }
    }
    else
    {
        //end wall climb (wall climb end checks for if doing some kinda wall climb movement)
        //WallClimbEnd();
    }

}

bool UAdvMovementComponent::CanWallClimb()
{
    //if is able to wall climb:

    //check if holding fwd, falling and isnt wall running then check wall climb
    if (GetForwardInput() > 0 && IsFalling())
    {
        // || CurrentAdvMoveState == EAdvancedMoveState::AMS_WallClimb
        if (CurrentAdvMoveState != EAdvancedMoveState::AMS_Prone && CurrentAdvMoveState != EAdvancedMoveState::AMS_Crouched)
        {
            //UE_LOG(LogAdvMovement, Display, TEXT("Can Check Wall climb"));
            return true;
        }
    }
    return false;
}

bool UAdvMovementComponent::WallClimbSweepCast(const FVector StartLocation, const FVector EndLocation)
{
    FHitResult hit;
    FCollisionQueryParams params;
    params.AddIgnoredActor(GetOwner());

//Debuggin
/*
#pragma region Debugs
DrawDebugSphere(GetWorld(), StartLocation, 10, 12, FColor::Green, false, 1.0f);
DrawDebugSphere(GetWorld(), EndLocation, 10, 12, FColor::Red, false, 1.0f);
DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Black, false, 1.0f);
#pragma endregion
*/

    //Wall Climb
    if (GetWorld()->SweepSingleByChannel(hit, StartLocation, EndLocation, FQuat::Identity, ECC_GameTraceChannel3, FCollisionShape::MakeSphere(20), params))
    {
        //save out distance to mantle
        mantleTraceDist = hit.Distance;

        if (!IsWalkable(hit))
        {
            //if something hit but not walkable
            return false;
        }
        else
        {
            mantlePos = FVector(hit.ImpactPoint.X, hit.ImpactPoint.Y, hit.ImpactPoint.Z + GetCharacterOwner()->GetDefaultHalfHeight());
            return true;
        }
    }
    return false;

}

FVector UAdvMovementComponent::CalculateWallClimbVector(const bool bIsEyes)
{
    if (bIsEyes)
    {
        FVector eyes;
        FRotator tmp;
        GetCharacterOwner()->GetController()->GetActorEyesViewPoint(eyes, tmp);
        eyes += FVector(0, 0, 50.f);
        eyes += GetCharacterOwner()->GetActorForwardVector() * 50.f;

        return eyes;
    }
    else
    {
        FVector feet = GetActorFeetLocation();
        feet.Z += 30.f;
        feet += GetCharacterOwner()->GetActorForwardVector() * 50.f;
        return feet;
    }
}

void UAdvMovementComponent::DoWallClimb()
{
    FHitResult hit;
    FCollisionQueryParams params;
    params.AddIgnoredActor(GetOwner());

    if (GetForwardInput() > 0 && GetWorld()->LineTraceSingleByChannel(hit, CalculateWallClimbVector(false), CalculateWallClimbVector(false) + GetCharacterOwner()->GetActorForwardVector() * 50.f,ECC_GameTraceChannel3,params))
    {
        if (!hit.GetActor())
        {
            WallClimbEnd();
            return;
        }

        if (hit.GetActor()->ActorHasTag(wallRunTag))
        {
            Server_UpdateMovementState(EAdvancedMoveState::AMS_WallClimb);

            //Tell Server to Launch Character D:
            wallClimbImpactNormal = hit.ImpactNormal;
            wallClimbImpactNormal *= -500.f;

            Server_LaunchCharacter(FVector(wallClimbImpactNormal.X, wallClimbImpactNormal.Y, wallClimbSpeed), true, true);
            Server_LaunchCharacter_Implementation(FVector(wallClimbImpactNormal.X, wallClimbImpactNormal.Y, wallClimbSpeed), true, true);
        }
    }
    else
    {
        WallClimbEnd();
    }

}

void UAdvMovementComponent::CheckMantleTick()
{
    //If holding Forward
    if (GetForwardInput() > 0)
    {
        Server_DoMantle(GetActorLocation(),mantlePos);
        Server_DoMantle_Implementation(GetActorLocation(), mantlePos);
    }
}

void UAdvMovementComponent::Server_DoMantle_Implementation(const FVector actorLocation, const FVector targetLocation)
{
    UE_LOG(LogAdvMovement, Display, TEXT("Mantling..."));
    
    //Make Server
    FVector targetLoc = UKismetMathLibrary::VInterpTo(actorLocation, targetLocation, GetWorld()->DeltaTimeSeconds, IsQuickMantle()?quickMantleSpeed:mantleSpeed);
    GetCharacterOwner()->SetActorLocation(targetLoc);

    if (UKismetMathLibrary::Vector_Distance(actorLocation, targetLocation) < 15.0f) 
    {
        CameraShake(IsQuickMantle() ? QuickMantleShake : MantleShake, 1.f);
        WallClimbEnd();
        bWantsToMantle = false;
    }
}

bool UAdvMovementComponent::IsQuickMantle()
{
    if (mantleTraceDist > GetCharacterOwner()->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight())
    {
        return true;
    }
    return false;
}


void UAdvMovementComponent::WallClimbEnd()
{
    if (CurrentAdvMoveState == EAdvancedMoveState::AMS_WallClimb || CurrentAdvMoveState == EAdvancedMoveState::AMS_WallHang || CurrentAdvMoveState == EAdvancedMoveState::AMS_Mantle)
    {
        
        UE_LOG(LogAdvMovement, Display, TEXT("Wall Climb ended"));
        if (CurrentAdvMoveState == EAdvancedMoveState::AMS_Mantle || PreviousAdvMoveState == EAdvancedMoveState::AMS_Mantle)
        {
            if (IsQuickMantle())
            {
                Server_UpdateMovementState(EAdvancedMoveState::AMS_Run);
            }
            else
            {
                Server_UpdateMovementState(EAdvancedMoveState::AMS_Walk);
            }
            bWantsToMantle = false;
        }
        else Server_UpdateMovementState(EAdvancedMoveState::AMS_Run);
        bIsAbleToWallClimb = false;
        GetWorld()->GetTimerManager().SetTimer(wallClimbSupressedTimer, this, &UAdvMovementComponent::ResetWallClimb, timeBetweenWallClimbs, false);
    }
}

void UAdvMovementComponent::ResetWallClimb()
{
    UE_LOG(LogAdvMovement, Display, TEXT("Reset Wall Climb"));
    GetWorld()->GetTimerManager().ClearTimer(wallClimbSupressedTimer);
    bIsAbleToWallClimb = true;
}

#pragma endregion

#pragma region Mantle




#pragma endregion


#pragma endregion

#pragma region Camera

void UAdvMovementComponent::CameraShake(const TSubclassOf<UCameraShakeBase> camShake, const float shakeScale)
{
    if (!GetCharacterOwner()->IsLocallyControlled() || !GetCharacterOwner()->HasActiveCameraComponent() || !camShake || !bMoveCamShake) return;

    if (GetWorld()->GetFirstLocalPlayerFromController())
    {
        GetWorld()->GetFirstLocalPlayerFromController()->PlayerController->ClientStartCameraShake(camShake, shakeScale);
    }
    else {UE_LOG(LogAdvMovement, Error, TEXT("No Player Controller found!")); return;}
}

#pragma endregion

float UAdvMovementComponent::GetAbsoluteMaxPlayerSpeed()
{
    TArray<float> speeds;
    speeds.Add(sprintMoveSpeed);
    speeds.Add(proneMoveSpeed);
    speeds.Add(wallRunMaxSpeed);
    speeds.Add(defaultWalkSpeed);
    speeds.Add(MaxWalkSpeedCrouched);
    speeds.Add(wallClimbSpeed);

    int index;
    float returnValue;
    UKismetMathLibrary::MaxOfFloatArray(speeds, index, returnValue);
    return returnValue;
}

void UAdvMovementComponent::ForceResetJumps()
{
    //UE_LOG(LogTemp, Warning, TEXT("Reset Jumps"));
    SetMovementMode(EMovementMode::MOVE_Flying);
    GetCharacterOwner()->ResetJumpState();
    SetMovementMode(EMovementMode::MOVE_Falling);
}



float UAdvMovementComponent::GetForwardInput()
{
    return UKismetMathLibrary::Dot_VectorVector(GetCharacterOwner()->GetActorForwardVector(), GetLastInputVector());
}
