// Fill out your copyright notice in the Description page of Project Settings.

#include "AICharacters/BaseCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Runtime/Engine/Classes/Components/SkeletalMeshComponent.h"
#include "AIController.h"
#include "Classes/Components/DecalComponent.h"
#include "Classes/Materials/Material.h"
#include "EngineUtils.h"
#include "AITypes.h"
#include "Containers/Queue.h"
#include "Engine/GameEngine.h"
#include "CharacterAttributesComponent.h"

// Sets default values
ABaseCharacter::ABaseCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CurrentLocIndex = 0;
	isPlayerCharacter = true;
	CharacterName = "NONE";
	ShouldLoop = false;

	GetMesh()->SetReceivesDecals(false);

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(38.f, 88.0f);

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 250.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	Attributes = CreateDefaultSubobject<UCharacterAttributesComponent>("Attributes");

	static ConstructorHelpers::FObjectFinder<UBlueprint> NodeVisActor(TEXT("Blueprint'/Game/Blueprints/NodeVisual_BP.NodeVisual_BP'"));

	if (NodeVisActor.Object)
	{
		NodeVisualizer = Cast<UClass>(NodeVisActor.Object->GeneratedClass);
	}

	SelectionDecal = CreateDefaultSubobject<UDecalComponent>("SelectionDecal");
	SelectionDecal->SetupAttachment(RootComponent);
	SelectionDecal->SetRelativeLocation(FVector(0.f, 0.f, -75.f));
	SelectionDecal->SetRelativeRotation(FRotator(90.f, 0.f, 0.f));
	SelectionDecal->DecalSize = FVector(65.f, 65.f, 65.f);
	SelectionDecal->bDestroyOwnerAfterFade = false;
	SelectionDecal->SetVisibility(false);

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Material(TEXT("MaterialInstanceConstant'/Game/Materials/DecalMats/SelectionDecalMatInst.SelectionDecalMatInst'"));

	if (Material.Object != NULL)
	{
		SelectionDecalMat = Cast<UMaterialInterface>(Material.Object);
		SelectionDecal->SetDecalMaterial(SelectionDecalMat);
	}

	TargetDecal = CreateDefaultSubobject<UDecalComponent>("TargetDecal");
	TargetDecal->SetupAttachment(RootComponent);
	TargetDecal->SetRelativeLocation(FVector(0.f, 0.f, -75.f));
	TargetDecal->SetRelativeRotation(FRotator(90.f, 0.f, 0.f));
	TargetDecal->DecalSize = FVector(85.f, 85.f, 85.f);
	TargetDecal->bDestroyOwnerAfterFade = false;
	TargetDecal->SetVisibility(false);

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Material2(TEXT("MaterialInstanceConstant'/Game/Materials/DecalMats/TargetDecalMatInst.TargetDecalMatInst'"));

	if (Material2.Object != NULL)
	{
		TargetDecalMat = Cast<UMaterialInterface>(Material2.Object);
		TargetDecal->SetDecalMaterial(TargetDecalMat);
	}
}

// Called when the game starts or when spawned
void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	GetMesh()->SetRenderCustomDepth(false);
	AICon = Cast<AAIController>(GetController());
	TArray<FString> Names = { "Bob","Joe", "Trevor", "Justin", "Jill", "Liana", "Vicky", "Judy", "Melony" };
	CharacterName = Names[FMath::RandRange(0, Names.Num() - 1)];
}

bool ABaseCharacter::PathToLocation(const FVector &loc)
{
	FAIMoveRequest locMoveRequest;
	locMoveRequest.SetAcceptanceRadius(0.0f);
	locMoveRequest.SetGoalLocation(loc);
	locMoveRequest.SetAllowPartialPath(true);
	locMoveRequest.SetCanStrafe(true);
	locMoveRequest.SetProjectGoalLocation(true);
	locMoveRequest.SetUsePathfinding(true);

	FPathFollowingRequestResult result = AICon->MoveTo(locMoveRequest, &CurrPath);

	if (result.Code != EPathFollowingRequestResult::RequestSuccessful || !CurrPath->IsValid())
	{
		ResetPath();
		return false;
	}

	CurrMoveID = result.MoveId.GetID();
	return true;
}

void ABaseCharacter::DrawPath()
{
	TArray<FNavPathPoint> pathPoints = CurrPath->GetPathPoints();
	if (pathPoints.Num() > 1)
	{
		for (int32 i = 0; i < pathPoints.Num(); ++i)
		{
			pathPoints[i].Location;
		}
		if (TargetLocationsArray.Num() == 1)
			AddNode(pathPoints[pathPoints.Num() - 1].Location);
	}
}

void ABaseCharacter::AddNode(const FVector &loc)
{
	if (NodeVisualizer == nullptr) return;

	FRotator Rotation(0.0f, 0.0f, 0.0f);
	FActorSpawnParameters SpawnInfo;
	NodeVisActors.Enqueue(GetWorld()->SpawnActor<AActor>(NodeVisualizer, loc, Rotation, SpawnInfo));
}

void ABaseCharacter::DeleteNodes()
{
	AActor* decaly;
	while (!NodeVisActors.IsEmpty())
	{
		NodeVisActors.Dequeue(decaly);
		decaly->Destroy();
	}
}

void ABaseCharacter::ResetPath()
{
	TargetLocationsArray.Empty();
	CurrentLocIndex = 0;
	DeleteNodes();
}

void ABaseCharacter::HandleActionChanges()
{

}

void ABaseCharacter::Move()
{
	if (AICon->GetMoveStatus() != EPathFollowingStatus::Idle || TargetLocationsArray.Num() == 0)
	{
		return;
	}
	if (CurrentLocIndex == TargetLocationsArray.Num() - 1 && ShouldLoop)
	{
		CurrentLocIndex = -1;
	}
	else if (CurrentLocIndex == TargetLocationsArray.Num() - 1)
	{
		ResetPath();
		return;
	}

	++CurrentLocIndex;
	if (!ShouldLoop)
	{
		AActor* decaly;
		NodeVisActors.Dequeue(decaly);
		decaly->Destroy();
	}

	if (PathToLocation(TargetLocationsArray[CurrentLocIndex]))
	{
		DrawPath();
	}
}

// Called every frame
void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Move();
}

// Called to bind functionality to input
void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ABaseCharacter::Selected_Implementation()
{
	GetMesh()->SetRenderCustomDepth(true);
	SelectionDecal->SetVisibility(true);
}

void ABaseCharacter::UnSelected_Implementation()
{
	GetMesh()->SetRenderCustomDepth(false);
	SelectionDecal->SetVisibility(false);
}

void ABaseCharacter::MoveToAction(const FVector &loc, const float &radious, const bool &addOn)
{
	if (AICon == nullptr) return;

	ShouldLoop = false;

	if (addOn)
	{
		if (!IsLocationCloseToFirst(loc))
		{
			TargetLocationsArray.Add(loc);
			AddNode(loc);
		}
		else if (TargetLocationsArray.Num() > 1)
		{
			ShouldLoop = true;
		}
	}
	else
	{
		ResetPath();
		TargetLocationsArray.Add(loc);
		DeleteNodes();
	}

	if (AICon->GetMoveStatus() != EPathFollowingStatus::Idle && addOn)
	{
		return;
	}

	if (PathToLocation(loc))
	{
		DrawPath();
	}

}

float ABaseCharacter::GetCharacterRadious()
{
	return GetCapsuleComponent()->GetUnscaledCapsuleRadius();
}

bool ABaseCharacter::IsLocationCloseToFirst(const FVector &locToCheck)
{
	if (TargetLocationsArray.Num() == 0) return false;
	if ((TargetLocationsArray[0] - locToCheck).SizeSquared() <= 80.f)
	{
		return true;
	}
	return false;
}