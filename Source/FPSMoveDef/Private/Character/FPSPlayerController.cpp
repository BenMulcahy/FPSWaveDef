// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/FPSPlayerController.h"
#include "..\..\Public\Character\FPSCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"

AFPSPlayerController::AFPSPlayerController()
{

}

void AFPSPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (playerCharacter = Cast<AFPSCharacter>(GetPawn())); //Get ref to player character
    else UE_LOG(LogPlayerController, Error, TEXT("No FPS Character Found"));


    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        UE_LOG(LogPlayerController, Display, TEXT("Mapping Context Set"));
        Subsystem->AddMappingContext(DefaultMappingContext, 0);
    }
}

void AFPSPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (TObjectPtr<UEnhancedInputComponent> EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent))
    {
        //Jump
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &AFPSPlayerController::CallJump);
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AFPSPlayerController::CallStopJump);

        //Weapon Swap
        EnhancedInputComponent->BindAction(NextWeaponAction, ETriggerEvent::Triggered, this, &AFPSPlayerController::CallNextWeapon);
        EnhancedInputComponent->BindAction(LastWeaponAction, ETriggerEvent::Triggered, this, &AFPSPlayerController::CallPreviousWeapon);

        //WeaponAim
        EnhancedInputComponent->BindAction(AimDownSights, ETriggerEvent::Started, this, &AFPSPlayerController::CallStartAim);
        EnhancedInputComponent->BindAction(AimDownSights, ETriggerEvent::Completed, this, &AFPSPlayerController::CallStopAim);
        EnhancedInputComponent->BindAction(AimDownSights, ETriggerEvent::Canceled, this, &AFPSPlayerController::CallStopAim);

        //WeaponShoot
        EnhancedInputComponent->BindAction(shootAction, ETriggerEvent::Started, this, &AFPSPlayerController::CallShoot);
        EnhancedInputComponent->BindAction(shootAction, ETriggerEvent::Completed, this, &AFPSPlayerController::CallStopShoot);
        EnhancedInputComponent->BindAction(shootAction, ETriggerEvent::Canceled, this, &AFPSPlayerController::CallStopShoot);

        //Switch Fire Mode
        EnhancedInputComponent->BindAction(changeFireAction, ETriggerEvent::Triggered, this, &AFPSPlayerController::CallChangeFireMode);

        //Reload
        EnhancedInputComponent->BindAction(reloadAction, ETriggerEvent::Triggered, this, &AFPSPlayerController::CallReload);

        //Crouching
        EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &AFPSPlayerController::CallCrouch);

        //Prone
        EnhancedInputComponent->BindAction(ProneAction, ETriggerEvent::Triggered, this, &AFPSPlayerController::CallProne);

        //Interact
        EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AFPSPlayerController::CallInteract);
        EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Completed, this, &AFPSPlayerController::CallInteractHeld);

        //Sprint
        EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Triggered, this, &AFPSPlayerController::CallSprint);

        //Moving
        EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AFPSPlayerController::Move);

        //Looking
        EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AFPSPlayerController::Look);


        UE_LOG(LogPlayerController, Display, TEXT("Enhanced Player Input Setup"));
    }
    else
    {
        UE_LOG(LogPlayerController, Error, TEXT("No Enhanced Input Component Found!"));
    }
}

void AFPSPlayerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AFPSPlayerController::CallJump()
{
    playerCharacter->Jump();
}

void AFPSPlayerController::CallStopJump()
{
    playerCharacter->StopJumping();
}

void AFPSPlayerController::CallNextWeapon()
{
    playerCharacter->NextWeapon();
}

void AFPSPlayerController::CallPreviousWeapon()
{
    playerCharacter->PreviousWeapon();
}

void AFPSPlayerController::CallStartAim()
{
    playerCharacter->StartAim();
}

void AFPSPlayerController::CallStopAim()
{
    playerCharacter->StopAim();
}

void AFPSPlayerController::CallShoot()
{
    playerCharacter->Shoot();
}

void AFPSPlayerController::CallStopShoot()
{
    playerCharacter->StopShooting();
}

void AFPSPlayerController::CallChangeFireMode()
{
    playerCharacter->ChangeFireMode();
}

void AFPSPlayerController::CallReload()
{
    playerCharacter->ReloadWeapon();
}

void AFPSPlayerController::CallCrouch()
{
    playerCharacter->DoCrouch();
}

void AFPSPlayerController::CallInteract()
{
    playerCharacter->Interact();
}

void AFPSPlayerController::CallInteractHeld()
{
    playerCharacter->InteractHold();
}

void AFPSPlayerController::CallProne()
{
    playerCharacter->Prone();
}

void AFPSPlayerController::CallSprint()
{
    playerCharacter->Sprint();
}

void AFPSPlayerController::Move(const FInputActionValue& Value)
{
    //UE_LOG(LogTemp, Warning, TEXT("Movement"));
    // input is a Vector2D
    FVector2D MovementVector = Value.Get<FVector2D>();
    // add movement 
    playerCharacter->AddMovementInput(playerCharacter->GetActorForwardVector(), MovementVector.Y);
    playerCharacter->AddMovementInput(playerCharacter->GetActorRightVector(), MovementVector.X);

}

void AFPSPlayerController::Look(const FInputActionValue& Value)
{
    // input is a Vector2D
    FVector2D LookAxisVector = Value.Get<FVector2D>();

    // add yaw and pitch input to controller
    playerCharacter->AddControllerYawInput(LookAxisVector.X);
    playerCharacter->AddControllerPitchInput(LookAxisVector.Y);

}
