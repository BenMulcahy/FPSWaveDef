// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WatchPointViewer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "AdvMovementComponent.generated.h"


/**
 * 
 */
class ACharacter;

//Delegates
UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStateChangeDelegate);

UENUM(BlueprintType)
enum class EAdvancedMoveState : uint8
{
	AMS_Walk UMETA(DisplayName = "Walking"),
	AMS_Run UMETA(DisplayName = "Running"),
	AMS_Crouched UMETA(DisplayName = "Crouched"),
	AMS_Prone UMETA(DisplayName = "Prone"),
	AMS_WallRun UMETA(DisplayName = "Wall Running"),
	AMS_WallClimb UMETA(DisplayName = "Wall Climbing"), //SETUP
	AMS_WallHang UMETA(DisplayName = "Wall Hanging"),//SETUP
	AMS_Dive UMETA(DisplayName = "Diving"),//SETUP
	AMS_Slide UMETA(DisplayName = "Sliding"),//SETUP
	AMS_Mantle UMETA(DisplayName = "Mantling"),//SETUP
};

UENUM(BlueprintType)
enum class EAdvWallRunState : uint8
{
	WR_null UMETA(DisplayName = "null"), WR_R UMETA(DisplayName = "Wall Run Right"), WR_L UMETA(DisplayName = "Wall Run Left")
};

UCLASS()
class FPSMOVEDEF_API UAdvMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
public:
	//Base Constructor
	UAdvMovementComponent();
/// <summary>
/// Overrides
/// </summary>
public:
	void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

#pragma region														 Properties
/// <summary>
///  Public UProperties
/// </summary>
public:
	UPROPERTY(BlueprintAssignable)
		FOnStateChangeDelegate OnStateChanged;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Movement | Character Movement: Parkour", DisplayName = "Can Parkour")
		bool bCanParkour = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Custom Movement | Character Movement: Parkour", DisplayName = "Wall Run Object Tag")
		FName wallRunTag;

	//Sprinting

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Movement | Character Movement: Sprinting", DisplayName = "Can Sprint")
		bool bCanSprint = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Movement | Character Movement: Sprinting")
		float sprintMoveSpeed = 700.f;

	//Wall running
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Movemenet | Character Movement: Wall Running", DisplayName = "Can Wall Run?")
		bool bCanWallRun;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Movement | Character Movement: Wall Running", DisplayName = "Use Gravity Wall Run?")
		bool bWallRunGravity = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Movement | Character Movement: Wall Running", DisplayName = "Minimum Time Between Wall Runs")
		float timeBetweenWallRuns = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Movement | Character Movement: Wall Running")
		float wallRunDuration = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Movement | Character Movement: Wall Running")
		float WallRunGravity = 0.25f;

	/// Added To sprint move speed when wall running
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Movement | Character Movement: Wall Running")
		float wallRunMoveSpeedModifier = 0.f;

	/// <summary>
/// Added to wallrunjumpHeight to allow for adjustment on base jump height
/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Movement | Character Movement: Wall Running")
		float wallRunJumpOffHeightModifier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Movement | Character Movement: Wall Running")
		float wallJumpOffForce = 300.f;

	//Wall Climb
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Movement | Character Movement: Wall Climbing", DisplayName = "Can Wall Climb")
		bool bCanWallClimb;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Movement | Character Movement: Wall Climbing")
		float timeBetweenWallClimbs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Movement | Character Movement: Wall Climbing")
		float wallClimbSpeed = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Movement | Character Movement: Wall Climbing")
		float wallClimbJumpOffHeightModifier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Movement | Character Movement: Wall Climbing", DisplayName = "Can Mantle")
		bool bCanMantle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Movement | Character Movement: Wall Climbing | Mantle")
		float mantleSpeed = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Movement | Character Movement: Wall Climbing | Mantle")
		float quickMantleSpeed = 20.f;

	//Prone

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Movement | Character Movement: Prone")
		float proneMoveSpeed = 100.f;

	//Crouched/Sliding

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Movement | Character Movement: Crouched | Sliding")
		float slideDuration = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Movement | Character Movement: Crouched | Sliding")
		float slideSpeedModifier = 50.f;

	//States

	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_MoveStateUpdated, Category = "Custom Movement | States")
		EAdvancedMoveState CurrentAdvMoveState = EAdvancedMoveState::AMS_Walk;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Custom Movement | States")
		EAdvancedMoveState PreviousAdvMoveState = EAdvancedMoveState::AMS_Walk;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Custom Movement | States")
		EAdvWallRunState WallRunState = EAdvWallRunState::WR_null;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Shakes", DisplayName = "Movment Camera Shakes Enabled?")
		bool bMoveCamShake = true;

/// <summary>
/// Private UProperties
/// </summary>
private:
	FTimerHandle parkourCheckTimer;
	float wallRunMaxSpeed;
	float wallRunJumpHeight;
	float wallClimbJumpHeight;
	float defaultWalkSpeed = 600.f;
	bool bValuesSet = false;
	FVector wallRunNormal;
	float defaultGravity;
	float slideSpeed;

	UPROPERTY(Replicated)
		bool bIsAbleToWallRun;

	UPROPERTY(Replicated)
		bool bWantsToMantle;

	UPROPERTY(Replicated)
		bool bIsAbleToWallClimb;

	FVector slideVector;
	float mantleTraceDist;
	FVector mantlePos;
	FVector wallClimbImpactNormal;

	//Timers
	FTimerHandle wallRunDurationTimer;
	FTimerHandle wallRunSupressedTimer;
	FTimerHandle wallClimbSupressedTimer;
	FTimerHandle slideDurationTimer;

/// <summary>
/// Protected UProperties
/// </summary>
protected:
	//Camera Shakes
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Camera Shakes", DisplayName = "Jump/Land")
		TSubclassOf<UCameraShakeBase> JumpLandShake;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Camera Shakes", DisplayName = "Ledge Grab")
		TSubclassOf<UCameraShakeBase> LedgeGrabShake;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Camera Shakes", DisplayName = "Mantle")
		TSubclassOf<UCameraShakeBase> MantleShake;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Camera Shakes", DisplayName = "Quick Mantle")
		TSubclassOf<UCameraShakeBase> QuickMantleShake;


#pragma endregion

#pragma region														Functions
/// <summary>
/// Public Functions
/// </summary>
public:
	UFUNCTION()
		virtual void Sprint();

	UFUNCTION()
		virtual void StartSprint();

	UFUNCTION()
		virtual void StopSprint();

	UFUNCTION()
		virtual void CheckCrouch();

	UFUNCTION()
		virtual void DoCrouch();

	UFUNCTION()
		virtual void StartCrouch();

	UFUNCTION()
		virtual void StopCrouch();

	UFUNCTION()
		virtual void Slide();

	UFUNCTION()
		virtual void StartSlide();

	UFUNCTION()
		virtual void StopSlide();

	UFUNCTION()
		virtual void Prone();

	UFUNCTION()
		virtual void StartProne();

	UFUNCTION()
		virtual void StopProne();

	UFUNCTION()
		virtual void OnRep_MoveStateUpdated();

	UFUNCTION(BlueprintCallable)
		virtual void OnLand();

	UFUNCTION(BlueprintCallable)
		virtual void WallRunEnd();

	UFUNCTION(BlueprintCallable)
		virtual void WallClimbEnd();

	UFUNCTION(BlueprintCallable)
		virtual void ResetWallRun();

	UFUNCTION(BlueprintCallable)
		virtual void ResetWallClimb();

	UFUNCTION(BlueprintCallable, Category = "Mantling")
		bool IsQuickMantle();

	//Return the absolute max speed the player can have
	UFUNCTION(BlueprintCallable, Category = "Values")
		float GetAbsoluteMaxPlayerSpeed();


	/// <summary>
	/// Allows for custom logic to be called if wanted on jump
	/// </summary>
	UFUNCTION()
		virtual void OnJump();

/// <summary>
/// Private Functions
/// </summary>
private:
	/// <summary>
	/// Handles all checks for parkour movement
	/// </summary>
	UFUNCTION()
		void ParkourTick();

	UFUNCTION()
		FVector CalculateWallRunVector(const bool bIsLeft = false);

	UFUNCTION()
		FVector CalculateWallClimbVector(const bool bIsEyes = false);

	UFUNCTION()
		virtual void DoWallRun(const FVector fwd, const FVector wall);

	UFUNCTION()
		virtual void DoWallClimb();

	UFUNCTION()
		virtual void CheckMantleTick();

	UFUNCTION()
		bool CheckSprint();

	UFUNCTION()
		void SetValues();

	UFUNCTION(BlueprintCallable)
	void CameraShake(const TSubclassOf<UCameraShakeBase> camShake, const float shakeScale);
	
	/// <summary>
	/// //Returns the forward input value as a float between -1 and 1
	/// </summary>
	/// <returns></returns>
	UFUNCTION()
		float GetForwardInput(); 

	UFUNCTION()
		void ForceResetJumps();

	UFUNCTION()
		void JumpFromWall();

protected:
	UFUNCTION()
		void WallRunTick();

	UFUNCTION()
		void WallClimbTick();

	UFUNCTION()
		bool CheckWallRun(const FVector lineTraceStartLocation, const FVector lineTraceEndLocation, float wallRunDir);

	UFUNCTION()
		bool CanWallClimb();

	UFUNCTION()
		bool WallClimbSweepCast(const FVector StartLocation, const FVector EndLocation);

#pragma region Server 
/// <summary>
/// Protected Functions
/// </summary>
public:
	UFUNCTION(Server, Reliable)
		virtual void Server_UpdateMovementState(const EAdvancedMoveState ToState);
	virtual void Server_UpdateMovementState_Implementation(const EAdvancedMoveState ToState);

	UFUNCTION(Server, Reliable)
		virtual void Server_WallJump();
	virtual FORCEINLINE void Server_WallJump_Implementation()
	{
		UE_LOG(LogTemp, Warning, TEXT("Jump Off Wall"));
		WallRunEnd();
		JumpFromWall();
	}

	//Call Launch character on the server
	UFUNCTION(Server, Reliable)
		virtual void Server_LaunchCharacter(FVector launchVector, bool bOverrideXY, bool bOverrideZ, bool bResetJumps = false);
	virtual FORCEINLINE void Server_LaunchCharacter_Implementation(FVector launchVector, bool bOverrideXY, bool bOverrideZ, bool bResetJumps = false)
	{
		GetCharacterOwner()->LaunchCharacter(launchVector, bOverrideXY, bOverrideZ);
		if (bResetJumps)
		{
			ForceResetJumps();
		}
	}

	UFUNCTION(Server, Reliable)
		void Server_DoMantle(const FVector actorLocation, const FVector targetLocation);
	virtual void Server_DoMantle_Implementation(const FVector actorLocation, const FVector targetLocation);



#pragma endregion
#pragma endregion

};
