// Fill out your copyright notice in the Description page of Project Settings.

#include "BoxHighlightWidget.h"

void UBoxHighlightWidget::GetNewPositionAndSize(const FVector2D &newMousePos, FVector2D & NewPos, FVector2D & NewSize)
{
	FVector2D smallPoint;
	smallPoint.X = (StartPosition.X < newMousePos.X) ? StartPosition.X : newMousePos.X;
	smallPoint.Y = (StartPosition.Y < newMousePos.Y) ? StartPosition.Y : newMousePos.Y;

	FVector2D largePoint;
	largePoint.X = (StartPosition.X > newMousePos.X) ? StartPosition.X : newMousePos.X;
	largePoint.Y = (StartPosition.Y > newMousePos.Y) ? StartPosition.Y : newMousePos.Y;

	NewPos = smallPoint;
	NewSize.X = largePoint.X - smallPoint.X;
	NewSize.Y = largePoint.Y - smallPoint.Y;
}