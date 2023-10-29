// Fill out your copyright notice in the Description page of Project Settings.


#include "Props/DoorInteractable.h"

#include "Character/FPSCharacter.h"
#include "Components/BoxComponent.h"


// Sets default values
ADoorInteractable::ADoorInteractable()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	root = CreateDefaultSubobject<USceneComponent>(TEXT("root"));
	RootComponent = root;

	DoorFrameMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door Frame"));
	DoorFrameMesh->SetupAttachment(root);

	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door"));
	DoorMesh->SetIsReplicated(true);
	DoorMesh->SetupAttachment(DoorFrameMesh);

	interactionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Interaction Zone"));
	interactionBox->SetCollisionProfileName(TEXT("OverlapOnlyPawn"));
	interactionBox->SetupAttachment(root);

	interactionBox->OnComponentBeginOverlap.AddDynamic(this, &ADoorInteractable::OnOverlapBegin);
	interactionBox->OnComponentEndOverlap.AddDynamic(this, &ADoorInteractable::OnOverlapEnd);
}


void ADoorInteractable::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(AFPSCharacter* character =  Cast<AFPSCharacter>(OtherActor))
	{
		character->InteractionEnum = EInteractionType::IT_Door;
		character->doorInteractable = this;
	}
}

void ADoorInteractable::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && OtherActor != this) 
	{
		if (AFPSCharacter* character = Cast<AFPSCharacter>(OtherActor))
		{
			character->InteractionEnum = EInteractionType::null;
			character->doorInteractable = nullptr;
		}
	}
}

void ADoorInteractable::OpenDoor_Implementation()
{
	//TODO: Tie in points
	 
	UE_LOG(LogTemp,Warning,TEXT("Door open"));
	//Open Door
	DoorMesh->SetRelativeRotation(DoorOpenedRotation);
	bOpened = true;
}
