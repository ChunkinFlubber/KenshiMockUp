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
#include "Player/PlayerHud.h"
#include "UI/BoxHighlightWidget.h"

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

}

// Called to bind functionality to input
void APlayerOverwatch::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

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