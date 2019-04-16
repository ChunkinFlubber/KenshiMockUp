// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CharacterAttributesComponent.generated.h"

class UCharacterAttributesComponent;

USTRUCT(BlueprintType)
struct FAttribute
{
	GENERATED_BODY()
private:
	typedef void(UCharacterAttributesComponent::*LevelUpFunctionPtr)(void);
	LevelUpFunctionPtr LevelUpFunction;
	UCharacterAttributesComponent* Owner;
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
		FName AttributeName = "None";
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
		int32 MaxLevel = 100;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
		int32 CurrentLevel = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
		int32 CurrentXP = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
		int32 NextLevelXP = 100;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
		float NextXPMult = 1.5f;

	FORCEINLINE void SetLevelUpFunc(void(UCharacterAttributesComponent::*callWhenLevelUp)(), UCharacterAttributesComponent* owner)
	{
		LevelUpFunction = callWhenLevelUp;
		Owner = owner;
	};

	FORCEINLINE bool AddXP(int32 XP)
	{
		XP = XP + CurrentXP;
		if (XP >= NextLevelXP)
		{
			while (NextLevelXP <= XP && CurrentLevel < MaxLevel)
			{
				XP = XP - NextLevelXP;
				++CurrentLevel;
				CurrentLevel = FMath::Min(CurrentLevel, MaxLevel);
				NextLevelXP = CurrentLevel * NextXPMult * 100;
			}
			CurrentXP = XP;
			if (Owner)
			{
				(Owner->*LevelUpFunction)();
			}
			return true;
		}
		CurrentXP = XP;
		return false;
	};

	FORCEINLINE void SetLevel(int32 desiredLevel)
	{
		CurrentLevel = FMath::Min(desiredLevel, MaxLevel);
		NextLevelXP = CurrentLevel * NextXPMult * 100;
		CurrentXP = 0;
		if (Owner)
		{
			(Owner->*LevelUpFunction)();
		}
	};

};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class KENBAT_API UCharacterAttributesComponent : public UActorComponent
{
	GENERATED_BODY()

private:

	class ABaseCharacter* Owner;
	class UCharacterMovementComponent* OwnerMovement;

public:
	// Sets default values for this component's properties
	UCharacterAttributesComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attribute")
		FAttribute DexAttribute;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		float MaxMoveSpeed;
	UFUNCTION(BlueprintCallable)
		void SetDexLevel(int32 level);
protected:

	virtual void DexLevelUp();
	virtual void DexCheck();
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


};