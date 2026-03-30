// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "MHGameplayAbility.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class EAbilityInputID : uint8
{
	None UMETA(DisplayName = "None"), // 0
	PrimaryAbility UMETA(DisplayName = "PrimaryAbility"), // 1
	SecondaryAbility UMETA(DisplayName = "SecondaryAbility"), // 2
	DefensiveAbility UMETA(DisplayName = "DefensiveAbility"), // 3
	MovementAbility UMETA(DisplayName = "MovementAbility") // 4

};

UCLASS(Blueprintable)
class MHPROTOTYPE_API UMHGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
public:

	UMHGameplayAbility();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	bool ShouldShowInAbilitiesBar = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activation")
	bool AutoActivateWhenGranted = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	EAbilityInputID AbilityInputID = EAbilityInputID::None;

	UFUNCTION(BlueprintCallable, Category = "Ability")
	void SetAbilityLevel(int32 NewLevel);

private:
	UFUNCTION(BlueprintCallable, Category = "Helpers")
	bool HasPlayerController() const;
};
