// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ScreenNamesWidget.generated.h"

/**
 *
 */
UCLASS()
class KENBAT_API UScreenNamesWidget : public UUserWidget
{
	GENERATED_BODY()
protected:
	class UPanelWidget* RootWidget;
	TArray<class ABaseCharacter*> PrevRenderedCharacters;
	TArray<class ABaseCharacter*> RenderedCharacters;
	void AddTextToScreen();
	void RemoveTextFromScreen();
	void MoveTextOnScreen();
public:
	UPROPERTY(BlueprintReadWrite)
		bool bHasPopulated;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		TSubclassOf<class UOnScreenText> OSTWidget;
	virtual void NativeConstruct() override;
	UFUNCTION(BlueprintCallable)
		void GetRenderedCharacters();
	UFUNCTION(BlueprintCallable)
		void PopulateCharacterTexts();
	UFUNCTION(BlueprintCallable)
		void UpdateCharacterTexts();
};
