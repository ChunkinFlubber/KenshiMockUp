// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BoxHighlightWidget.generated.h"

/**
 *
 */
UCLASS()
class KENBAT_API UBoxHighlightWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BoxHighlight")
		FVector2D StartPosition;

	UFUNCTION(BlueprintImplementableEvent, Category = "BoxHighlight")
		void RenderBoxHighlight(ESlateVisibility vis, bool shouldRender);

	UFUNCTION(BlueprintImplementableEvent, Category = "BoxHighlight")
		void UpdateBoxHighlight();

protected:
	UFUNCTION(BlueprintCallable, Category = "BoxHighlight")
		void GetNewPositionAndSize(const FVector2D &newMousePos, FVector2D &outNewPos, FVector2D &outNewSize);
};