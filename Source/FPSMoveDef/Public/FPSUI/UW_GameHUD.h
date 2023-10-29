// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_GameHUD.generated.h"

/**
 * 
 */
UCLASS()
class FPSMOVEDEF_API UUW_GameHUD : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
		TObjectPtr<class AFPSPlayerController> playerController;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
		TObjectPtr<class AFPSCharacter> fpsPlayer;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
		TObjectPtr<class ARangedWeapon> currentWeaponEquipped;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
		TObjectPtr<class AWaveDefGamemode> waveDefGM;

private:

protected:

	virtual void NativeOnInitialized() override;
};
