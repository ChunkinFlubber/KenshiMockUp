// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "KenBatGameMode.h"
#include "KenBatCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Player/PlayerOverwatch.h"

AKenBatGameMode::AKenBatGameMode()
{
	// set default pawn class to our Blueprinted character
	/*static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}*/

	DefaultPawnClass = APlayerOverwatch::StaticClass();
}
