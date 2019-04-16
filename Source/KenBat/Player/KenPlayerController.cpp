// Fill out your copyright notice in the Description page of Project Settings.

#include "KenPlayerController.h"
#include "UI/BoxHighlightWidget.h"
#include "Player/PlayerOverwatch.h"
#include "AICharacters/BaseCharacter.h"
#include "Engine/GameEngine.h"
#include "EngineUtils.h"
#include "CollisionQueryParams.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "DrawDebugHelpers.h"
#include "UI/ScreenNamesWidget.h"
#include "UI/StatsWidget.h"
#include "KenBat.h"

AKenPlayerController::AKenPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
	isSelecting = false;
	StartPoint = FVector2D::ZeroVector;
	isRenderingBox = false;
	isAddingMode = false;

	static ConstructorHelpers::FClassFinder<UBoxHighlightWidget> BoxWidgetBPClass(TEXT("/Game/UI/SelectionBoxWidget"));
	if (BoxWidgetBPClass.Class != NULL)
	{
		BHWidget = BoxWidgetBPClass.Class;
	}

	static ConstructorHelpers::FClassFinder<UScreenNamesWidget> ScreenNameWidgetBPClass(TEXT("/Game/UI/OnScreenNamesWidget"));
	if (ScreenNameWidgetBPClass.Class != NULL)
	{
		SNWidget = ScreenNameWidgetBPClass.Class;
	}

	static ConstructorHelpers::FClassFinder<UStatsWidget> StatsWidgetBPClass(TEXT("/Game/UI/CharacterStatsWidget"));
	if (StatsWidgetBPClass.Class != NULL)
	{
		CSWidget = StatsWidgetBPClass.Class;
	}
}

void AKenPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	InputComponent->BindAction("AdditionMode", IE_Pressed, this, &AKenPlayerController::AdditionMode);
	InputComponent->BindAction("AdditionMode", IE_Released, this, &AKenPlayerController::RegularMode);
	InputComponent->BindAction("Select", IE_Pressed, this, &AKenPlayerController::SelectStart);
	InputComponent->BindAction("Select", IE_Released, this, &AKenPlayerController::SelectTest);
	InputComponent->BindAction("PawnAction", IE_Released, this, &AKenPlayerController::PawnAction);
}

void AKenPlayerController::BeginPlay()
{
	Super::BeginPlay();
	FInputModeGameAndUI inputMode;
	inputMode.SetHideCursorDuringCapture(false);
	SetInputMode(inputMode);
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

	if (BHWidget)
	{
		BoxHighlightWidget = CreateWidget<UBoxHighlightWidget>(this, BHWidget);
		if (BoxHighlightWidget)
		{
			BoxHighlightWidget->AddToViewport(1);
		}
	}

	if (SNWidget)
	{
		SceenNamesWidget = CreateWidget<UScreenNamesWidget>(this, SNWidget);
		if (SceenNamesWidget)
		{
			SceenNamesWidget->AddToViewport(0);
		}
	}

	if (CSWidget)
	{
		CharacterStatsWidget = CreateWidget<UStatsWidget>(this, CSWidget);
		if (CharacterStatsWidget)
		{
			CharacterStatsWidget->AddToViewport(2);
		}
	}
}

void AKenPlayerController::Tick(float deltaTime)
{
	Super::Tick(deltaTime);
	UpdateSelect();
}

void AKenPlayerController::AdditionMode()
{
	isAddingMode = true;
}

void AKenPlayerController::RegularMode()
{
	isAddingMode = false;
}

void AKenPlayerController::SelectStart()
{
	GEngine->GameViewport->GetMousePosition(StartPoint);
	isSelecting = true;
}

void AKenPlayerController::UpdateSelect()
{
	if (isSelecting)
	{
		FVector2D mouse;
		GEngine->GameViewport->GetMousePosition(mouse);
		bool sizeBigEnough = (StartPoint - mouse).SizeSquared() > 56;
		if (!isRenderingBox && sizeBigEnough)
		{
			BoxHighlightWidget->RenderBoxHighlight(ESlateVisibility::HitTestInvisible, true);
			isRenderingBox = true;
		}
		else
		{
			BoxHighlightWidget->UpdateBoxHighlight();
		}
	}
}

void AKenPlayerController::SelectTest()
{
	isSelecting = false;
	FVector2D EndPoint;
	GEngine->GameViewport->GetMousePosition(EndPoint);

	if (!isRenderingBox) SingleSelect();
	else
	{
		BoxSelection(EndPoint);
		BoxHighlightWidget->RenderBoxHighlight(ESlateVisibility::Hidden, false);
		isRenderingBox = false;
	}
}

void AKenPlayerController::SingleSelect()
{
	FHitResult hitStuff;
	TArray< TEnumAsByte < EObjectTypeQuery > > searchObjects;
	searchObjects.Add(EObjectTypeQuery::ObjectTypeQuery3);//Pawn
	//searchObjects.Add(EObjectTypeQuery::ObjectTypeQuery7);//JobStuffs??
	//searchObjects.Add(EObjectTypeQuery::ObjectTypeQuery8);//Buildings??

	GetHitResultUnderCursorForObjects(searchObjects, true, hitStuff);

	if (!isAddingMode) UnSelectPawns();

	if (!hitStuff.bBlockingHit)
	{
		return;
	}


	ABaseCharacter* AIpawn = Cast<ABaseCharacter>(hitStuff.Actor);
	if (AIpawn != nullptr)
	{
		if (isAddingMode && SelectedCharacters.Contains(AIpawn))
		{
			SelectedCharacters.Remove(AIpawn);
			AIpawn->UnSelected();
			CharacterStatsWidget->RemoveOldCharacter();
		}
		else
		{
			SelectedCharacters.AddUnique(AIpawn);
			AIpawn->Selected();
			CharacterStatsWidget->ReciveCharacter(AIpawn);
		}
	}
}

void AKenPlayerController::BoxSelection(const FVector2D &endPoint)
{
	if (!isAddingMode) UnSelectPawns();

	TArray<ABaseCharacter*> renderedActors;
	GetRenderedCharacters(renderedActors);
	TArray<ABaseCharacter*> selectedActors;
	CheckCharactersScreenLoc(renderedActors, selectedActors, endPoint);

	ABaseCharacter* AIpawn;
	for (int32 i = 0; i < selectedActors.Num(); ++i)
	{
		AIpawn = selectedActors[i];
		if (AIpawn != nullptr)
		{
			if (isAddingMode && SelectedCharacters.Contains(AIpawn))
			{
				SelectedCharacters.Remove(AIpawn);
				AIpawn->UnSelected();
			}
			else
			{
				SelectedCharacters.AddUnique(AIpawn);
				AIpawn->Selected();
			}
		}
	}
	if (SelectedCharacters.Num() > 0)
		CharacterStatsWidget->ReciveCharacter(SelectedCharacters[0]);
}

void AKenPlayerController::UnSelectPawns()
{
	for (int32 i = 0; i < SelectedCharacters.Num(); ++i)
	{
		SelectedCharacters[i]->UnSelected();
	}
	SelectedCharacters.Empty(10);
	CharacterStatsWidget->RemoveOldCharacter();
}

void AKenPlayerController::PawnAction()
{
	if (SelectedCharacters.Num() == 0) return;

	FHitResult hitStuff;
	TArray< TEnumAsByte < EObjectTypeQuery > > searchObjects;
	searchObjects.Add(UCollisionProfile::Get()->ConvertToObjectType(ECC_WorldStatic));//Terrain
	//earchObjects.Add(UCollisionProfile::Get()->ConvertToObjectType(ECC_WorldStatic));//Pawn for enemy to fight
	//searchObjects.Add(EObjectTypeQuery::ObjectTypeQuery7);//JobStuffs??

	GetHitResultUnderCursorForObjects(searchObjects, true, hitStuff);

	if (!hitStuff.bBlockingHit)
	{
		return;
	}
	if (hitStuff.Component->GetCollisionObjectType() == ECC_WorldStatic)
	{
		MoveAction(hitStuff.Location);
	}

}

void AKenPlayerController::MoveAction(const FVector &loc)
{
	if (SelectedCharacters.Num() == 1)
	{
		SelectedCharacters[0]->MoveToAction(loc, 0.0f, isAddingMode);
	}
	else
	{
		int32 numCharacters = SelectedCharacters.Num();
		int32 numXs = (int32)floor(sqrt(numCharacters));
		int32 numYs = (int32)ceil(sqrt(numCharacters));
		bool niceAmount = numXs * numYs >= numCharacters;
		if (!niceAmount) numXs += 1;
		float characterSize = SelectedCharacters[0]->GetCharacterRadious() * 2;
		float amountToTrans = characterSize + 20.f;
		float oddAddition = (numYs % 2 == 0) ? characterSize * .5f : 0;

		float startingY = amountToTrans * (float)floor(numYs * .5f) - oddAddition + loc.Y;
		FVector squarePos = loc;
		squarePos.Y = startingY;

		/*for (int32 i = 0; i < numCharacters; ++i)
		{
			SelectedCharacters[i]->MoveToAction(squarePos, 0.0f, isAddingMode);
			squarePos.Y -= amountToTrans;
		}*/

		int32 index = 0;
		for (int32 x = 0; x < numXs; ++x)
		{
			for (int32 y = 0; y < numYs && index < numCharacters; ++y)
			{
				//DrawDebugPoint(GetWorld(), squarePos, 35, FColor::Blue, true);
				SelectedCharacters[index]->MoveToAction(squarePos, 0.0f, isAddingMode);
				squarePos.Y -= amountToTrans;
				++index;
			}

			squarePos.X -= amountToTrans;
			if (!niceAmount && x == numXs - 2)
			{
				int32 amountLeft = numCharacters - index;
				squarePos.Y = amountToTrans * (float)floor(amountLeft * .5f) - ((amountLeft % 2 == 0) ? characterSize * .5f : 0) + loc.Y;
			}
			else
			{
				squarePos.Y = startingY;
			}
		}
	}
}

void AKenPlayerController::GetRenderedCharacters(TArray<ABaseCharacter*>& charArray)
{
	charArray.Empty();

	for (TActorIterator<ABaseCharacter> itr(GetWorld()); itr; ++itr)
	{
		if (itr->WasRecentlyRendered())
		{
			charArray.Add(*itr);
		}
	}
}

void AKenPlayerController::CheckCharactersScreenLoc(const TArray<ABaseCharacter*>& charArray, TArray<ABaseCharacter*>& charInBox, FVector2D endPoint)
{
	FBox2D SelectionBox;

	SelectionBox.Max = FVector2D((StartPoint.X > endPoint.X) ? StartPoint.X : endPoint.X, (StartPoint.Y > endPoint.Y) ? StartPoint.Y : endPoint.Y);
	SelectionBox.Min = FVector2D((StartPoint.X < endPoint.X) ? StartPoint.X : endPoint.X, (StartPoint.Y < endPoint.Y) ? StartPoint.Y : endPoint.Y);

	FVector2D screenLoc;

	for (int32 i = 0; i < charArray.Num(); ++i)
	{
		ProjectWorldLocationToScreen(charArray[i]->GetActorLocation(), screenLoc);
		if (SelectionBox.IsInside(screenLoc))
		{
			charInBox.Add(charArray[i]);
		}
	}
}