// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OnScreenText.generated.h"

/**
 *
 */
UCLASS()
class KENBAT_API UOnScreenText : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void SetOnScreenText(const FText& text);
};
