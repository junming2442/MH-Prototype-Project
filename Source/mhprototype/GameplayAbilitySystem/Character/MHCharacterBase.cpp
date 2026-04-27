// Fill out your copyright notice in the Description page of Project Settings.


#include "MHCharacterBase.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "mhprototype/GameplayAbilitySystem/MHAbilitySystemComponent.h"
#include "mhprototype/GameplayAbilitySystem/AttributeSets/BasicAttributeSet.h"
#include "mhprototype/GameplayAbilitySystem/AttributeSets/CombatAttributeSet.h"
#include "mhprototype/GameplayAbilitySystem/Abilities/MHGameplayAbility.h"

// Sets default values
AMHCharacterBase::AMHCharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create Ability System Component
	AbilitySystemComponent = CreateDefaultSubobject<UMHAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(AscReplicationMode);

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate

	// Add basic attribute set
	BasicAttributeSet = CreateDefaultSubobject<UBasicAttributeSet>(TEXT("BasicAttributeSet"));

	// Add combat attribute set
	CombatAttributeSet = CreateDefaultSubobject<UCombatAttributeSet>(TEXT("CombatAttributeSet"));

}

// Called when the game starts or when spawned
void AMHCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	AbilitySystemComponent->RegisterGameplayTagEvent(FGameplayTag::RequestGameplayTag("State.Dead"))
		.AddUObject(this, &AMHCharacterBase::OnDeadTagChanged);

}

// Called every frame
void AMHCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AMHCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AMHCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		GrantAbilities(StartingAbilities);
	}

	if (AbilitySystemComponent)
	{
		// Bind the Character's function to the Component's delegate
		AbilitySystemComponent->OnAbilityEnded.AddUObject(this, &AMHCharacterBase::HandleAbilityEnded);
	}
}

void AMHCharacterBase::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
}

UAbilitySystemComponent* AMHCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

TArray<FGameplayAbilitySpecHandle> AMHCharacterBase::GrantAbilities(TArray<TSubclassOf<UGameplayAbility>> AbilitiesToGrant, const int32 Level)
{
	if (!AbilitySystemComponent || !HasAuthority())
	{
		return TArray<FGameplayAbilitySpecHandle>();
	}
	
	TArray <FGameplayAbilitySpecHandle> AbilityHandles;
	for (TSubclassOf<UGameplayAbility> Ability : AbilitiesToGrant)
	{
		int32 InputID = -1;
		bool ShouldActivate = false;

		if (const UMHGameplayAbility* NexusAbilityCDO = GetDefault<UMHGameplayAbility>(Ability))
		{
			InputID = static_cast<int32>(NexusAbilityCDO->AbilityInputID);
			ShouldActivate = NexusAbilityCDO->AutoActivateWhenGranted;
		}

		FGameplayAbilitySpecHandle SpecHandle = AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(Ability, Level, InputID, this));

		AbilityHandles.Add(SpecHandle);

		if (ShouldActivate)
		{
			AbilitySystemComponent->TryActivateAbility(SpecHandle);
		}
	}

	SendAbilitiesChangedEvent();
	return AbilityHandles;
}

void AMHCharacterBase::RemoveAbilities(TArray<FGameplayAbilitySpecHandle> AbilityHandlesToRemove)
{
	if (!AbilitySystemComponent || !HasAuthority())
	{
		return;
	}

	for (FGameplayAbilitySpecHandle AbilityHandle : AbilityHandlesToRemove)
	{
		AbilitySystemComponent->ClearAbility(AbilityHandle);
	}

	SendAbilitiesChangedEvent();
}

void AMHCharacterBase::SendAbilitiesChangedEvent()
{
	FGameplayEventData EventData;
	EventData.EventTag = FGameplayTag::RequestGameplayTag(FName("Event.Abilities.Changed"));
	EventData.Instigator = this;
	EventData.Target = this;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, EventData.EventTag, EventData);
}

void AMHCharacterBase::ServerSendGameplayEventToSelf_Implementation(FGameplayEventData EventData)
{
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, EventData.EventTag, EventData);
}

void AMHCharacterBase::HandleDeath_Implementation()
{
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->DisableMovement();

	FVector Impulse = GetActorForwardVector() * -20000;
	Impulse.Z = 15000;
	GetMesh()->AddImpulseAtLocation(Impulse, GetActorLocation());
}

void AMHCharacterBase::OnDeadTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	if (NewCount > 0)
	{
		HandleDeath();
	}
}

void AMHCharacterBase::HandleAbilityEnded_Implementation(const FAbilityEndedData& AbilityEndedData)
{
	// 1. We need the Ability System Component to look up the handle
	if (!AbilitySystemComponent) return;

	// 2. Find the "Spec" (the data) using the Handle from the event
	FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromHandle(AbilityEndedData.AbilitySpecHandle);

	if (AbilitySpec && AbilitySpec->Ability)
	{
		// 3. Get the actual Ability object
		UGameplayAbility* EndedAbility = AbilitySpec->Ability;

		// 4. Check for your Monster Hunter attack tag
		FGameplayTag AttackTag = FGameplayTag::RequestGameplayTag(FName("GameplayAbility.MeleeAttack"));

		// Use HasTag to check if it's an attack or a sub-tag (like Ability.Attack.Heavy)
		//if (EndedAbility->AbilityTags.HasTag(AttackTag))
		if (EndedAbility->GetAssetTags().HasTag(AttackTag))
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Attack Ended!"));

			// Your logic: reset rotation lock, enable next combo window, etc.
		}
	}
}


