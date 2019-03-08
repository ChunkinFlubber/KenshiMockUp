// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseCharacter.generated.h"

UCLASS()
class KENBAT_API ABaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseCharacter();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class UDecalComponent* SelectionDecal;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class UMaterialInterface* SelectionDecalMat;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class UDecalComponent* TargetDecal;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class UMaterialInterface* TargetDecalMat;

	FNavPathSharedPtr CurrPath;
	int32 CurrMoveID;
	class AAIController* AICon;
	FVector TargetLoc;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		void Selected();
	virtual void Selected_Implementation();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		void UnSelected();
	virtual void UnSelected_Implementation();

	void MoveToLocation(FVector loc, float radious);
};
