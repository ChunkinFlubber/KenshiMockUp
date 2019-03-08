// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PlayerOverwatch.generated.h"

UCLASS()
class KENBAT_API APlayerOverwatch : public APawn
{
	GENERATED_BODY()

		UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class USceneComponent* RotationComp;

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* PlayerCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		class APlayerHud* MyHUD;

	struct FTimerHandle TerrainTimer;

	float TerrainLocationHeight;

	bool bAtZoom;

	bool bOrbiting;

	class APlayerController* PC;

public:
	// Sets default values for this pawn's properties
	APlayerOverwatch();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Camera)
		float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Camera)
		float BaseLookUpRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Camera)
		float MaxMovementSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Camera)
		float MinMovementSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Camera)
		float MovementSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Camera)
		float TerrainFollowSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Camera)
		float MaxZoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Camera)
		float MinZoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Camera)
		float Zoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Camera)
		float TargetZoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Camera)
		float TargetZoomAlpha;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Camera)
		float ZoomAlpha;

	/** Height to trace for terrain, used in terrain following */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Camera)
		float TerrainSearchHeight;

	/** Height to trace for terrain, used in terrain following */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Camera)
		AActor* FollowTarget;

protected:
	// Called each frame
	virtual void Tick(float deltaTime) override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);
	void Turn(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);
	void LookUp(float Rate);

	void GetTerrainLocation();

	/** Called via Timer for aligning the center to the terrain */
	void AlignToTerrain();

	void AdjustZoom(float Rate);

	void DoMovement();
	void DoZoom();

	void OrbitOn();
	void OrbitOff();

public:
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return PlayerCamera; }

};
