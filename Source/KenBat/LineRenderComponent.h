// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/PrimitiveComponent.h"
#include "LineRenderComponent.generated.h"

/**
 *
 */
UCLASS()
class KENBAT_API ULineRenderComponent : public UPrimitiveComponent
{
	GENERATED_BODY()
public:

	ULineRenderComponent(const FObjectInitializer& ObjectInit);

	/** Color used to draw the line. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LineRendererComponent)
		FColor LineColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LineRendererComponent, meta = (ClampMin = "0", UIMin = "0"))
		float LineThickness;

	/** Starting point of the line. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LineRendererComponent)
		FVector StartPoint;

	/** End point of the line. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LineRendererComponent)
		FVector EndPoint;

	/** True if multiple lines are going to be drawn. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LineRendererComponent)
		bool bMultipleLines;

	/** True if the lines are going to be separate. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LineRendererComponent)
		bool bNoneContinuousLines;

	/** Line points, if NoneContinuousLines then it goes {Start,End,Start,End,...}. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LineRendererComponent)
		TArray<FVector> LinePoints;

	/** If true it allows Collision when placing even if collision is not enabled. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LineRendererComponent)
		bool bDrawOnlyIfSelected;

	UFUNCTION(BlueprintCallable, DisplayName = "SetLineColor", Category = "Components|LineRenderer")
		virtual void SetLineColor(FLinearColor NewColor);

	UFUNCTION(BlueprintCallable, DisplayName = "SetLineStartPoint", Category = "Components|LineRenderer")
		virtual void SetLineStartPoint(FVector NewStart);

	UFUNCTION(BlueprintCallable, DisplayName = "SetLineEndPoint", Category = "Components|LineRenderer")
		virtual void SetLineEndPoint(FVector NewEnd);

	UFUNCTION(BlueprintCallable, DisplayName = "AddLinePoint", Category = "Components|LineRenderer")
		virtual void AddLinePoint(FVector NewPoint);

	UFUNCTION(BlueprintCallable, DisplayName = "AddLine", Category = "Components|LineRenderer")
		virtual void AddLine(FVector NewStartPoint, FVector NewEndPoint);

	UFUNCTION(BlueprintCallable, DisplayName = "InsertLinePoint", Category = "Components|LineRenderer")
		virtual void InsertLinePoint(FVector NewPoint, int index);

	UFUNCTION(BlueprintCallable, DisplayName = "RemoveLinePoint", Category = "Components|LineRenderer")
		virtual void RemoveLinePoint(int index);

	virtual void Draw(FPrimitiveDrawInterface* PDI);

	virtual bool ShouldDraw();

public:
	//~ Begin UPrimitiveComponent Interface.
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	//~ End UPrimitiveComponent Interface.
	virtual bool ShouldRecreateProxyOnUpdateTransform() const override;
};
