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

// Sets default values
ABaseCharacter::ABaseCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GetMesh()->SetReceivesDecals(false);

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(38.f, 88.0f);

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 250.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

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
}

// Called every frame
void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (TargetDecal->IsVisible())
	{
		if (AICon->GetMoveStatus() == EPathFollowingStatus::Idle)
			TargetDecal->SetVisibility(false);
		else
			TargetDecal->SetWorldLocation(TargetLoc);
	}
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
	if (AICon->GetMoveStatus() != EPathFollowingStatus::Idle)
		TargetDecal->SetVisibility(true);
}

void ABaseCharacter::UnSelected_Implementation()
{
	GetMesh()->SetRenderCustomDepth(false);
	SelectionDecal->SetVisibility(false);
	TargetDecal->SetVisibility(false);
}

void ABaseCharacter::MoveToLocation(FVector loc, float radious)
{
	if (AICon == nullptr) return;

	FAIMoveRequest locMoveRequest;
	locMoveRequest.SetAcceptanceRadius(radious);
	locMoveRequest.SetGoalLocation(loc);
	locMoveRequest.SetAllowPartialPath(true);
	locMoveRequest.SetCanStrafe(true);
	locMoveRequest.SetProjectGoalLocation(true);
	locMoveRequest.SetUsePathfinding(true);

	FPathFollowingRequestResult result = AICon->MoveTo(locMoveRequest, &CurrPath);

	if (result.Code != EPathFollowingRequestResult::RequestSuccessful || !CurrPath->IsValid()) return;

	CurrMoveID = result.MoveId.GetID();

	TArray<FNavPathPoint> pathPoints = CurrPath->GetPathPoints();
	for (int32 i = 0; i < pathPoints.Num(); ++i)
	{

	}
	if (pathPoints.Num() > 1)
	{
		TargetLoc = pathPoints[pathPoints.Num() - 1];
		TargetDecal->SetWorldLocation(TargetLoc);
		TargetDecal->SetVisibility(true);
	}

	//AICon->MoveToLocation(loc, radious, true, true, true);
}