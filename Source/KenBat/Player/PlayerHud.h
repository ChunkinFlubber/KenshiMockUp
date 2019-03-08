// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "PlayerHud.generated.h"

/**
 *
 */
UCLASS()
class KENBAT_API APlayerHud : public AHUD
{
	GENERATED_BODY()

		virtual void DrawHUD() override;
};