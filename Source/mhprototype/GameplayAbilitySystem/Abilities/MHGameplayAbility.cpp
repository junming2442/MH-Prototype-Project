// Fill out your copyright notice in the Description page of Project Settings.


#include "mhprototype/GameplayAbilitySystem/Abilities/MHGameplayAbility.h"

UMHGameplayAbility::UMHGameplayAbility()
{
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("GameplayAbility.Active")));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Dead")));
}

bool UMHGameplayAbility::HasPlayerController() const
{
	const APawn* PawnObject = Cast<APawn>(GetAvatarActorFromActorInfo());
	if (!PawnObject)
	{
		return false;
	}
	return PawnObject->GetController()->IsA<APlayerController>();
}
