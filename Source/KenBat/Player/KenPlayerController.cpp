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

AKenPlayerController::AKenPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
	isSelecting = false;
	StartPoint = FVector2D::ZeroVector;
	isRenderingBox = false;
	isRegularMode = true;

	static ConstructorHelpers::FClassFinder<UBoxHighlightWidget> BoxWidgetBPClass(TEXT("/Game/UI/SelectionBoxWidget"));
	if (BoxWidgetBPClass.Class != NULL)
	{
		BHWidget = BoxWidgetBPClass.Class;
	}
	//BHWidget = UBoxHighlightWidget::StaticClass();
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
			BoxHighlightWidget->AddToViewport();
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
	isRegularMode = false;
}

void AKenPlayerController::RegularMode()
{
	isRegularMode = true;
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

	if (isRegularMode) UnSelectPawns();

	if (!hitStuff.bBlockingHit)
	{
		return;
	}


	ABaseCharacter* AIpawn = Cast<ABaseCharacter>(hitStuff.Actor);
	if (AIpawn != nullptr)
	{
		if (!isRegularMode && SelectedCharacters.Contains(AIpawn))
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

void AKenPlayerController::BoxSelection(const FVector2D &endPoint)
{
	if (isRegularMode) UnSelectPawns();

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
			if (!isRegularMode && SelectedCharacters.Contains(AIpawn))
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
}

void AKenPlayerController::UnSelectPawns()
{
	for (int32 i = 0; i < SelectedCharacters.Num(); ++i)
	{
		SelectedCharacters[i]->UnSelected();
	}
	SelectedCharacters.Empty(10);
}

void AKenPlayerController::PawnAction()
{
	if (SelectedCharacters.Num() == 0) return;

	FHitResult hitStuff;
	TArray< TEnumAsByte < EObjectTypeQuery > > searchObjects;
	searchObjects.Add(UCollisionProfile::Get()->ConvertToObjectType(ECC_WorldStatic));//Pawn
	//searchObjects.Add(EObjectTypeQuery::ObjectTypeQuery7);//JobStuffs??
	//searchObjects.Add(EObjectTypeQuery::ObjectTypeQuery8);//Buildings??

	GetHitResultUnderCursorForObjects(searchObjects, true, hitStuff);

	if (!hitStuff.bBlockingHit)
	{
		return;
	}

	for (int32 i = 0; i < SelectedCharacters.Num(); ++i)
	{
		SelectedCharacters[i]->MoveToLocation(hitStuff.Location, 0.2f);
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

