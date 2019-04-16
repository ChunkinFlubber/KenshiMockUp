// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameEngine.h"

#define DEBUGMESSAGE(x) if(GEngine){GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, x);}