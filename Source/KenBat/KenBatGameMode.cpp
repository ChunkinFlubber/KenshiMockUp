// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "KenBatGameMode.h"
#include "KenBatCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Player/PlayerOverwatch.h"
#include "Player/PlayerHud.h"
#include "Player/KenPlayerController.h"

AKenBatGameMode::AKenBatGameMode()
{
	// set default pawn class to our Blueprinted character
	/*static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}*/

	DefaultPawnClass = APlayerOverwatch::StaticClass();
	HUDClass = APlayerHud::StaticClass();
	/*static ConstructorHelpers::FClassFinder<AKenPlayerController> PlayerConBPClass(TEXT("/Game/Blueprints/KenPlayerController_BP"));
	if (PlayerConBPClass.Class != NULL)
	{
		PlayerControllerClass = PlayerConBPClass.Class;
	}*/
	PlayerControllerClass = AKenPlayerController::StaticClass();
}
