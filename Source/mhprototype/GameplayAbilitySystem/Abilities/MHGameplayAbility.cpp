// Fill out your copyright notice in the Description page of Project Settings.


#include "mhprototype/GameplayAbilitySystem/Abilities/MHGameplayAbility.h"

UMHGameplayAbility::UMHGameplayAbility()
{
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("GameplayAbility.Active")));
}
