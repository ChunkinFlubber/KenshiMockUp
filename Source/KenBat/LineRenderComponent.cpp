// Fill out your copyright notice in the Description page of Project Settings.

#include "LineRenderComponent.h"
#include "PrimitiveSceneProxy.h"
#include "SceneManagement.h"

ULineRenderComponent::ULineRenderComponent(const FObjectInitializer& ObjectInit) : Super(ObjectInit)
{
	LineColor = FColor(255, 0, 0, 255);
	StartPoint = FVector(0, 0, 0);
	EndPoint = FVector(100, 0, 0);
	LineThickness = 1.0f;
	bHiddenInGame = true;
	SetGenerateOverlapEvents(false);
	bMultipleLines = false;
	bNoneContinuousLines = false;
	bDrawOnlyIfSelected = false;
	bUseEditorCompositing = true;
}

FPrimitiveSceneProxy * ULineRenderComponent::CreateSceneProxy()
{
	class FLineSceneProxy : public FPrimitiveSceneProxy
	{
	public:
		SIZE_T GetTypeHash() const override
		{
			static size_t UniquePointer;
			return reinterpret_cast<size_t>(&UniquePointer);
		}

		FLineSceneProxy(ULineRenderComponent* Component)
			: FPrimitiveSceneProxy(Component)
			, myLineRendComp(Component)
		{
			bWillEverBeLit = false;
		}

		virtual void GetDynamicMeshElements(
			const TArray<const FSceneView*>& Views,
			const FSceneViewFamily& ViewFamily,
			uint32 VisibilityMap,
			class FMeshElementCollector& Collector) const override
		{
			for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
			{
				if (VisibilityMap & (1 << ViewIndex))
				{
					const FSceneView* View = Views[ViewIndex];
					FPrimitiveDrawInterface* PDI = Collector.GetPDI(ViewIndex);
					if (!PDI)
					{
						continue;
					}

					//Draw to PDI
					myLineRendComp->Draw(PDI);
				}
			}
		}

		virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
		{
			const bool bProxyVisible = myLineRendComp->ShouldDraw();

			FPrimitiveViewRelevance Result;
			Result.bDrawRelevance = (IsShown(View) && bProxyVisible);
			Result.bDynamicRelevance = true;
			Result.bShadowRelevance = false;
			Result.bEditorPrimitiveRelevance = UseEditorCompositing(View);
			return Result;
		}

		virtual uint32 GetMemoryFootprint(void) const override { return(sizeof(*this) + GetAllocatedSize()); }
		uint32 GetAllocatedSize(void) const { return(FPrimitiveSceneProxy::GetAllocatedSize()); }

	private:
		ULineRenderComponent* myLineRendComp;
	};
	return new FLineSceneProxy(this);
}

bool ULineRenderComponent::ShouldRecreateProxyOnUpdateTransform() const
{
	return true;
}

void ULineRenderComponent::SetLineColor(FLinearColor NewColor)
{
	LineColor = NewColor.ToFColor(false);
}

void ULineRenderComponent::SetLineStartPoint(FVector NewStart)
{
	StartPoint = NewStart;
}

void ULineRenderComponent::SetLineEndPoint(FVector NewEnd)
{
	EndPoint = NewEnd;
}

void ULineRenderComponent::AddLinePoint(FVector NewPoint)
{
	LinePoints.Add(NewPoint);
}

void ULineRenderComponent::AddLine(FVector NewStartPoint, FVector NewEndPoint)
{
	LinePoints.Add(NewStartPoint);
	LinePoints.Add(NewEndPoint);
}

void ULineRenderComponent::InsertLinePoint(FVector NewPoint, int index)
{
	LinePoints.Insert(NewPoint, index);
}

void ULineRenderComponent::RemoveLinePoint(int index)
{
	if (index < LinePoints.Num())
		LinePoints.RemoveAt(index);
}

void ULineRenderComponent::Draw(FPrimitiveDrawInterface * PDI)
{
	FVector RelLoc = GetRelativeTransform().GetLocation();
	FVector ActLoc = GetOwner()->GetActorTransform().GetLocation();
	ActLoc = ActLoc + RelLoc;
	if (!bMultipleLines)
		PDI->DrawLine(StartPoint + ActLoc, EndPoint + ActLoc, FColor(LineColor), 0, LineThickness);
	else
	{
		if (LinePoints.Num() == 0)
			return;
		for (int i = 0; i < LinePoints.Num() - 1; ++i)
		{
			PDI->DrawLine(LinePoints[i] + ActLoc, LinePoints[i + 1] + ActLoc, FColor(LineColor), 0, LineThickness);
			if (bNoneContinuousLines)
				++i;
		}
	}
}

bool ULineRenderComponent::ShouldDraw()
{
	return (bDrawOnlyIfSelected) ? GetOwner()->IsSelectedInEditor() : true;
}