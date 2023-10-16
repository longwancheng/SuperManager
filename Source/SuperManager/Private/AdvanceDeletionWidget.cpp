// Fill out your copyright notice in the Description page of Project Settings.

#include "AdvanceDeletionWidget.h"
#include "Widgets/Layout/SScrollBox.h"
void SAdvanceDeletionTab::Construct(const FArguments& InArgs)
{
	bCanSupportFocus = true;
	
	StoreAssetsDataUnderSelectedFolderArray = InArgs._AssetsDataToStore;

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
				+ SHorizontalBox::Slot()
				[
					SNew(SListView<TSharedPtr<FAssetData>>)
					.ItemHeight(24.f)
					.ListItemsSource(&StoreAssetsDataUnderSelectedFolderArray)
					.OnGenerateRow(this,&SAdvanceDeletionTab::OnGenerateRowForList)
				]
			]
			+SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SScrollBox)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
			]
		];
}

TSharedRef<ITableRow> SAdvanceDeletionTab::OnGenerateRowForList(TSharedPtr<FAssetData> AssetDataToDisplay, const TSharedRef<STableViewBase>& OwnerTable)
{

	const FString DisplayAssetName = AssetDataToDisplay->AssetName.ToString();

	TSharedRef<STableRow<TSharedPtr<FAssetData>>> ListViewRowWidget = SNew(STableRow<TSharedPtr<FAssetData>>, OwnerTable)
		[
			SNew(STextBlock)
			.Text(FText::FromString(DisplayAssetName))
		];

	return ListViewRowWidget;
}
