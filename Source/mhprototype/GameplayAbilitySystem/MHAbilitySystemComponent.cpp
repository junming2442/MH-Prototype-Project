// Fill out your copyright notice in the Description page of Project Settings.

#include "mhprototype/GameplayAbilitySystem/MHAbilitySystemComponent.h"
#include "mhprototype/GameplayAbilitySystem/Character/MHCharacterBase.h"

UMHAbilitySystemComponent::UMHAbilitySystemComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UMHAbilitySystemComponent::BeginPlay()
{

}

void UMHAbilitySystemComponent::OnRep_ActivateAbilities()
{
	Super::OnRep_ActivateAbilities();

	AMHCharacterBase *Character = Cast<AMHCharacterBase>(GetOwner());
	if (!Character) return;

	bool bAbilitiesChanged = false;
	if (LastActivatableAbilities.Num() != ActivatableAbilities.Items.Num())
	{
		bAbilitiesChanged = true;
	}
	else
	{
		for (int32 i = 0; i < LastActivatableAbilities.Num(); i++)
		{
			if (LastActivatableAbilities[i].Ability != ActivatableAbilities.Items[i].Ability)
			{
				bAbilitiesChanged = true;
				break;
			}
		}
	}

	if (bAbilitiesChanged)
	{
		Character->SendAbilitiesChangedEvent();
		LastActivatableAbilities = ActivatableAbilities.Items;
	}
}

void UMHAbilitySystemComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}