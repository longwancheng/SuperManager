// Fill out your copyright notice in the Description page of Project Settings.

#include "AdvanceDeletionWidget.h"
#include "Widgets/Layout/SScrollBox.h"
void SAdvanceDeletionTab::Construct(const FArguments& InArgs)
{
	bCanSupportFocus = true;

	FSlateFontInfo TitleTextFont = FCoreStyle::Get().GetFontStyle(FName("EmbossedText"));

	TitleTextFont.Size = 30;

	ChildSlot
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("Advance Deletion")))
				.Font(TitleTextFont)
				.Justification(ETextJustify::Center)
				.ColorAndOpacity(FColor::White)
			]
			+SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
			]
			+SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SScrollBox)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SScrollBox)
			]
		];
}
