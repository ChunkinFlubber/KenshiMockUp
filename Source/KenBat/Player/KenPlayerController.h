// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "KenPlayerController.generated.h"

/**
 *
 */
UCLASS()
class KENBAT_API AKenPlayerController : public APlayerController
{
	GENERATED_BODY()
private:
	TArray< class ABaseCharacter*> SelectedCharacters;

	bool isSelecting;
	FVector2D StartPoint;
	bool isRenderingBox;
	bool isRegularMode;

	void AdditionMode();
	void RegularMode();
	void SelectStart();
	void UpdateSelect();
	void SelectTest();

	void SingleSelect();
	void BoxSelection(const FVector2D &endPoint);

	void UnSelectPawns();

	void PawnAction();

	void GetRenderedCharacters(TArray<ABaseCharacter*> &charArray);
	void CheckCharactersScreenLoc(const TArray<ABaseCharacter*>& charArray, TArray<ABaseCharacter*>& charInBox, FVector2D endPoint);
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widget)
		TSubclassOf<class UBoxHighlightWidget> BHWidget;
	class UBoxHighlightWidget* BoxHighlightWidget;

	virtual void SetupInputComponent() override;
	virtual void Tick(float deltaTime) override;
	virtual void BeginPlay() override;

public:
	AKenPlayerController();

};
