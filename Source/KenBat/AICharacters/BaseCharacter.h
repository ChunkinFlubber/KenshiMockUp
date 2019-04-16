// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseCharacter.generated.h"

class AAIController;

UCLASS()
class KENBAT_API ABaseCharacter : public ACharacter
{
	GENERATED_BODY()

private:
	bool IsLocationCloseToFirst(const FVector &locToCheck);
	bool PathToLocation(const FVector &loc);
	void DrawPath();
	void AddNode(const FVector &loc);
	void DeleteNodes();
	void ResetPath();
public:
	// Sets default values for this character's properties
	ABaseCharacter();

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		FString CharacterName;

	bool isPlayerCharacter;
	UPROPERTY(BlueprintReadWrite)
		class UOnScreenText* ScreenName;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class UDecalComponent* SelectionDecal;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class UMaterialInterface* SelectionDecalMat;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class UCharacterAttributesComponent* Attributes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSubclassOf<AActor> NodeVisualizer;

	TQueue<AActor*> NodeVisActors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class UDecalComponent* TargetDecal;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class UMaterialInterface* TargetDecalMat;

	FNavPathSharedPtr CurrPath;
	int32 CurrMoveID;
	AAIController* AICon;
	int32 CurrentLocIndex;
	TArray<FVector> TargetLocationsArray;
	bool ShouldLoop;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void HandleActionChanges();

	virtual void Move();

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

	virtual void MoveToAction(const FVector &loc, const float &radious, const bool &addedNode);

	float GetCharacterRadious();

	FORCEINLINE AAIController* GetAIController() { return AICon; };
};
