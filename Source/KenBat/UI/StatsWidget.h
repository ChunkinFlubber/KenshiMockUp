// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StatsWidget.generated.h"

/**
 *
 */
UCLASS()
class KENBAT_API UStatsWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void ReciveCharacter(class ABaseCharacter* character);
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void RemoveOldCharacter();
};