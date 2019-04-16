// Fill out your copyright notice in the Description page of Project Settings.

#include "CharacterAttributesComponent.h"
#include "BaseCharacter.h"
#include "AIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KenBat.h"

// Sets default values for this component's properties
UCharacterAttributesComponent::UCharacterAttributesComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.2f;
	// ...
	MaxMoveSpeed = 1200.f;
	DexAttribute.AttributeName = "DEX";
}


void UCharacterAttributesComponent::SetDexLevel(int32 level)
{
	DexAttribute.SetLevel(level);
}

void UCharacterAttributesComponent::DexLevelUp()
{
	if (OwnerMovement == nullptr) return;
	OwnerMovement->MaxWalkSpeed = 400 + ((float)DexAttribute.CurrentLevel / (float)DexAttribute.MaxLevel) * MaxMoveSpeed;
}

void UCharacterAttributesComponent::DexCheck()
{
	if (Owner == nullptr) return;
	if (Owner->GetAIController()->GetMoveStatus() == EPathFollowingStatus::Moving)
	{
		DexAttribute.AddXP(20);
	}
}

// Called when the game starts
void UCharacterAttributesComponent::BeginPlay()
{
	Super::BeginPlay();
	Owner = Cast<ABaseCharacter>(GetOwner());
	if (Owner == nullptr) return;
	OwnerMovement = Owner->GetCharacterMovement();
	DexAttribute.SetLevelUpFunc(&UCharacterAttributesComponent::DexLevelUp, this);
}


// Called every frame
void UCharacterAttributesComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (Owner == nullptr) return;

	DexCheck();
}