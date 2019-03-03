// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/PlayerOverwatch.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "CollisionQueryParams.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/GameFramework/PlayerController.h"
#include "AICharacters/BaseCharacter.h"
#include "Engine/GameEngine.h"
#include "EngineUtils.h"
#include "Runtime/Engine/Classes/GameFramework/HUD.h"

// Sets default values
APlayerOverwatch::APlayerOverwatch()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;
	MinMovementSpeed = 15.f;
	MaxMovementSpeed = 55.f;
	MovementSpeed = 45.f;
	TerrainFollowSpeed = 15.f;
	MaxZoom = 2500.f;
	MinZoom = 350.f;
	Zoom = MinZoom;
	TargetZoom = MinZoom;
	TargetZoomAlpha = 0.f;
	ZoomAlpha = 0.f;
	bAtZoom = false;
	bOrbiting = false;
	isSelecting = false;
	StartPoint = FVector2D::ZeroVector;

	FollowTarget = nullptr;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	RotationComp = CreateDefaultSubobject<USceneComponent>(TEXT("RotationComponent"));
	RotationComp->SetupAttachment(RootComponent);

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = MinZoom; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller
	CameraBoom->bDoCollisionTest = false;

	// Create a follow camera
	PlayerCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("PlayerCamera"));
	PlayerCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	PlayerCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
}

void APlayerOverwatch::Tick(float deltaTime)
{
	if (FollowTarget != nullptr)
	{
		SetActorLocation(FollowTarget->GetActorLocation());
	}
	else
	{
		AlignToTerrain();
	}

	DoMovement();
	DoZoom();
}

// Called when the game starts or when spawned
void APlayerOverwatch::BeginPlay()
{
	Super::BeginPlay();
	PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	GetTerrainLocation();
	GetWorldTimerManager().SetTimer(TerrainTimer, this, &APlayerOverwatch::GetTerrainLocation, 0.15f, true);
	FInputModeGameAndUI inputMode;
	inputMode.SetHideCursorDuringCapture(false);
	PC->SetInputMode(inputMode);
	PC->bShowMouseCursor = true;
	PC->bEnableClickEvents = true;
	PC->bEnableMouseOverEvents = true;
}

// Called to bind functionality to input
void APlayerOverwatch::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Select", IE_Pressed, this, &APlayerOverwatch::SelectStart);
	PlayerInputComponent->BindAction("Select", IE_Released, this, &APlayerOverwatch::SelectTest);
	PlayerInputComponent->BindAction("PawnAction", IE_Released, this, &APlayerOverwatch::PawnAction);

	PlayerInputComponent->BindAction("Orbit", IE_Pressed, this, &APlayerOverwatch::OrbitOn);
	PlayerInputComponent->BindAction("Orbit", IE_Released, this, &APlayerOverwatch::OrbitOff);

	PlayerInputComponent->BindAxis("MoveForward", this, &APlayerOverwatch::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APlayerOverwatch::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as a keyboard button
	PlayerInputComponent->BindAxis("Turn", this, &APlayerOverwatch::Turn);
	PlayerInputComponent->BindAxis("TurnRate", this, &APlayerOverwatch::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APlayerOverwatch::LookUp);
	PlayerInputComponent->BindAxis("LookUpRate", this, &APlayerOverwatch::LookUpAtRate);
	PlayerInputComponent->BindAxis("Zoom", this, &APlayerOverwatch::AdjustZoom);
}


void APlayerOverwatch::TurnAtRate(float Rate)
{
	if (!bOrbiting) return;
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void APlayerOverwatch::Turn(float Rate)
{
	if (!bOrbiting) return;
	AddControllerYawInput(Rate);
}

void APlayerOverwatch::LookUpAtRate(float Rate)
{
	if (!bOrbiting) return;
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void APlayerOverwatch::LookUp(float Rate)
{
	if (!bOrbiting) return;
	AddControllerPitchInput(Rate);
}

void APlayerOverwatch::GetTerrainLocation()
{
	if (FollowTarget != nullptr) return;

	FHitResult OutHit;

	FVector Start = GetActorLocation() + FVector(0, 0, 15000);
	FVector End = GetActorLocation() + FVector(0, 0, -15000);

	FCollisionQueryParams CollisionParams;

	GetWorld()->LineTraceSingleByObjectType(OutHit, Start, End, ECC_WorldStatic, CollisionParams);

	if (!OutHit.bBlockingHit)
	{
		return;
	}
	TerrainLocationHeight = OutHit.Location.Z + 50.f;
}

void APlayerOverwatch::AlignToTerrain()
{
	if (FollowTarget != nullptr) return;

	float alpha = GetWorld()->GetDeltaSeconds() * TerrainFollowSpeed;
	float locationDelta = (TerrainLocationHeight - GetActorLocation().Z) * alpha;
	AddActorWorldOffset({ 0.f,0.f,locationDelta });
}

void APlayerOverwatch::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		if (FollowTarget != nullptr)
		{
			FollowTarget = nullptr;
			SetActorTickEnabled(false);
		}

		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		//AddActorWorldOffset(Direction * Value * MovementSpeed);
		AddMovementInput(Direction, Value * MovementSpeed);
	}
}

void APlayerOverwatch::MoveRight(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		if (FollowTarget != nullptr)
		{
			FollowTarget = nullptr;
			SetActorTickEnabled(false);
		}

		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		//AddActorWorldOffset(Direction * Value * MovementSpeed);
		AddMovementInput(Direction, Value * MovementSpeed);
	}
}

void APlayerOverwatch::AdjustZoom(float Value)
{
	if (Value == 0.f) return;
	Zoom = CameraBoom->TargetArmLength;
	ZoomAlpha = 0.f;
	TargetZoomAlpha = FMath::Clamp(TargetZoomAlpha + Value * 0.05f, 0.f, 1.f);
	TargetZoom = FMath::Lerp(MinZoom, MaxZoom, TargetZoomAlpha);
	MovementSpeed = FMath::Lerp(MinMovementSpeed, MaxMovementSpeed, TargetZoomAlpha);
	bAtZoom = false;
}

void APlayerOverwatch::DoMovement()
{
	FVector move = ConsumeMovementInputVector();
	move.GetClampedToMaxSize(MovementSpeed);
	AddActorWorldOffset(move);
}

void APlayerOverwatch::DoZoom()
{
	if (bAtZoom) return;

	ZoomAlpha = FMath::Clamp(ZoomAlpha + GetWorld()->GetDeltaSeconds() * 5.f, 0.f, 1.f);
	CameraBoom->TargetArmLength = FMath::Lerp(Zoom, TargetZoom, ZoomAlpha);
}

void APlayerOverwatch::OrbitOn()
{
	bOrbiting = true;
}

void APlayerOverwatch::OrbitOff()
{
	bOrbiting = false;
}

void APlayerOverwatch::SelectStart()
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Start Selection"));
	PC->GetMousePosition(StartPoint.X, StartPoint.Y);
	isSelecting = true;
}

void APlayerOverwatch::SelectTest()
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("End Selection"));
	isSelecting = false;
	FVector2D EndPoint;
	PC->GetMousePosition(EndPoint.X, EndPoint.Y);

	bool boxSelecting = false;
	if ((EndPoint - StartPoint).SizeSquared() > 150.f) boxSelecting = true;

	if (!boxSelecting) SingleSelect();
	else BoxSelection(EndPoint);
}

void APlayerOverwatch::SingleSelect()
{
	FHitResult hitStuff;
	TArray< TEnumAsByte < EObjectTypeQuery > > searchObjects;
	searchObjects.Add(EObjectTypeQuery::ObjectTypeQuery3);//Pawn
	//searchObjects.Add(EObjectTypeQuery::ObjectTypeQuery7);//JobStuffs??
	//searchObjects.Add(EObjectTypeQuery::ObjectTypeQuery8);//Buildings??

	PC->GetHitResultUnderCursorForObjects(searchObjects, true, hitStuff);


	if (!hitStuff.bBlockingHit)
	{
		UnSelectPawns();
		return;
	}


	ABaseCharacter* AIpawn = Cast<ABaseCharacter>(hitStuff.Actor);
	if (AIpawn != nullptr && !SelectedCharacters.Contains(AIpawn))
	{
		UnSelectPawns();
		SelectedCharacters.Add(AIpawn);
		AIpawn->Selected();
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, hitStuff.Actor->GetName());
	}
}

void APlayerOverwatch::BoxSelection(const FVector2D &endPoint)
{
	UnSelectPawns();
	TArray<AActor*> selectedActors;
	PC->GetHUD()->GetActorsInSelectionRectangle(ABaseCharacter::StaticClass(), StartPoint, endPoint, selectedActors, true, false);

	for (int32 i = 0; i < selectedActors.Num(); ++i)
	{
		ABaseCharacter* AIpawn = Cast<ABaseCharacter>(selectedActors[i]);
		if (AIpawn != nullptr && !SelectedCharacters.Contains(AIpawn))
		{
			SelectedCharacters.Add(AIpawn);
			AIpawn->Selected();
			//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, AIpawn->GetName());
		}
	}
}

void APlayerOverwatch::UnSelectPawns()
{
	for (int32 i = 0; i < SelectedCharacters.Num(); ++i)
	{
		SelectedCharacters[i]->UnSelected();
	}
	SelectedCharacters.Empty(10);
}

void APlayerOverwatch::PawnAction()
{
	if (SelectedCharacters.Num() == 0) return;

	FHitResult hitStuff;
	TArray< TEnumAsByte < EObjectTypeQuery > > searchObjects;
	//UCollisionProfile::Get()->ConvertToObjectType(ECC_WorldStatic);
	searchObjects.Add(UCollisionProfile::Get()->ConvertToObjectType(ECC_WorldStatic));//Pawn
	//searchObjects.Add(EObjectTypeQuery::ObjectTypeQuery7);//JobStuffs??
	//searchObjects.Add(EObjectTypeQuery::ObjectTypeQuery8);//Buildings??

	PC->GetHitResultUnderCursorForObjects(searchObjects, true, hitStuff);

	if (!hitStuff.bBlockingHit)
	{
		return;
	}

	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, hitStuff.Location.ToString());

	for (int32 i = 0; i < SelectedCharacters.Num(); ++i)
	{
		SelectedCharacters[i]->MoveToLocation(hitStuff.Location, -1);
	}
}
