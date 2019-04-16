// Fill out your copyright notice in the Description page of Project Settings.

#include "ScreenNamesWidget.h"
#include "AICharacters/BaseCharacter.h"
#include "EngineUtils.h"
#include "OnScreenText.h"
#include "KenBat.h"
#include "Components/PanelWidget.h"
#include "Blueprint/WidgetTree.h"


void UScreenNamesWidget::NativeConstruct()
{
	bHasPopulated = false;
	Super::NativeConstruct();
	RootWidget = Cast<UPanelWidget>(GetRootWidget());
	RenderedCharacters.Empty(6);
	PrevRenderedCharacters.Empty(6);

	if (OSTWidget.GetDefaultObject() == nullptr)
	{
		OSTWidget = UOnScreenText::StaticClass();
	}
}

void UScreenNamesWidget::GetRenderedCharacters()
{
	PrevRenderedCharacters = RenderedCharacters;
	RenderedCharacters.Empty(6);

	for (TActorIterator<ABaseCharacter> itr(GetWorld()); itr; ++itr)
	{
		if (itr->WasRecentlyRendered() && itr->isPlayerCharacter)
		{
			RenderedCharacters.Add(*itr);
		}
	}
}

void UScreenNamesWidget::PopulateCharacterTexts()
{
	for (TActorIterator<ABaseCharacter> itr(GetWorld()); itr; ++itr)
	{
		if (itr->ScreenName == nullptr && itr->isPlayerCharacter)
		{
			FString textWidgetName = "Name Text of " + itr->GetName();
			UOnScreenText* textWidget = CreateWidget<UOnScreenText>(GetWorld()->GetFirstPlayerController(), OSTWidget);//WidgetTree->ConstructWidget<UOnScreenText>(OSTWidget, *textWidgetName);//CreateWidget<UOnScreenText>(GetWorld()->GetFirstPlayerController(), OSTWidget);
			if (textWidget == nullptr) return;
			RootWidget->AddChild(textWidget);
			textWidget->SetOnScreenText(FText::FromString(itr->CharacterName));
			itr->ScreenName = textWidget;
		}
	}
	bHasPopulated = true;
}

void UScreenNamesWidget::UpdateCharacterTexts()
{
	GetRenderedCharacters();
	AddTextToScreen();
	RemoveTextFromScreen();
	MoveTextOnScreen();
}

void UScreenNamesWidget::AddTextToScreen()
{
	//loop through rendered and see if prev rendered
	for (int32 i = 0; i < RenderedCharacters.Num(); ++i)
	{
		if (!PrevRenderedCharacters.Contains(RenderedCharacters[i]))
		{
			if (RenderedCharacters[i]->ScreenName)
				RenderedCharacters[i]->ScreenName->AddToViewport();
		}
	}
}

void UScreenNamesWidget::RemoveTextFromScreen()
{
	//loop through prev rendered and see if still rendered
	for (int32 i = 0; i < PrevRenderedCharacters.Num(); ++i)
	{
		if (!RenderedCharacters.Contains(PrevRenderedCharacters[i]))
		{
			PrevRenderedCharacters[i]->ScreenName->RemoveFromViewport();
		}
	}
}

void UScreenNamesWidget::MoveTextOnScreen()
{
	//move all curr rendered to currect screen pos
	for (int32 i = 0; i < RenderedCharacters.Num(); ++i)
	{
		FVector2D screenPos;
		GetWorld()->GetFirstPlayerController()->ProjectWorldLocationToScreen(RenderedCharacters[i]->GetActorLocation() + FVector(0, 0, 150.f), screenPos);
		RenderedCharacters[i]->ScreenName->SetPositionInViewport(screenPos);
	}
}


