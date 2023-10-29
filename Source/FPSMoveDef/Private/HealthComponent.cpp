// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	currentHealth = maxHealth;
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHealthComponent, currentHealth);
}

void UHealthComponent::TakeDamage(const float damage)
{
	currentHealth -= damage;
	if(GetOwner()->HasAuthority()) OnRep_CheckHealth();
}

void UHealthComponent::DoDestroy()
{
	GetOwner()->Destroy();
}

void UHealthComponent::OnRep_CheckHealth()
{
	if (currentHealth <= 0)
	{
		//UE_LOG(LogTemp, Error, TEXT("KILL MEEE"));
		GetOwner()->Destroy();
	}
	else
	{
		//UE_LOG(LogTemp, Warning, TEXT("Ow that hurt :( current health: %f"), currentHealth);
	}
}


