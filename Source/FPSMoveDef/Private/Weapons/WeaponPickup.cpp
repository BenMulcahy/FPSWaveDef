// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/WeaponPickup.h"
#include "Components/DecalComponent.h"
#include "Components/BoxComponent.h"
#include "..\..\Public\Character\FPSCharacter.h"

// Sets default values
AWeaponPickup::AWeaponPickup()
{
	PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;

    root = CreateDefaultSubobject<USceneComponent>(TEXT("root"));
    RootComponent = root;

    weaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Weapon Mesh"));
    weaponMesh->SetupAttachment(root);

    weaponDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("Weapon Wall Decal"));
    weaponDecal->SetupAttachment(root);

    interactionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Interaction Zone"));
    interactionBox->SetCollisionProfileName(TEXT("OverlapOnlyPawn"));
    interactionBox->SetupAttachment(root);

    interactionBox->OnComponentBeginOverlap.AddDynamic(this, &AWeaponPickup::OnOverlapBegin);
    interactionBox->OnComponentEndOverlap.AddDynamic(this, &AWeaponPickup::OnOverlapEnd);
}

void AWeaponPickup::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor != this) 
    {
        if (AFPSCharacter* character = Cast<AFPSCharacter>(OtherActor))
        {
            character->weaponPickup = this;
            character->InteractionEnum = EInteractionType::IT_Weapon;
            //UE_LOG(LogTemp, Display, TEXT("Began Overlap with : % s"), *OtherActor->GetName());
        }
    }
}

void AWeaponPickup::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor && OtherActor != this)
    {
        if (AFPSCharacter* character = Cast<AFPSCharacter>(OtherActor))
        {
            character->weaponPickup = NULL;
            character->InteractionEnum = EInteractionType::null;
            //UE_LOG(LogTemp, Display, TEXT("Ended Overlap with: %s"), *OtherActor->GetName());
        }
    }
}

void AWeaponPickup::BeginPlay()
{
    Super::BeginPlay();

    if(weaponMesh) weaponMesh->SetVisibility(false);
}






