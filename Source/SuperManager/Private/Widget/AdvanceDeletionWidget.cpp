// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/AdvanceDeletionWidget.h"
#include "Widgets/Layout/SScrollBox.h"
#include "DebugHeader.h"
#include "SlateBasics.h"
#include "SuperManager.h"
void SAdvanceDeletionTab::Construct(const FArguments& InArgs)
{
	bCanSupportFocus = true;
	
	StoreAssetsDataUnderSelectedFolderArray = InArgs._AssetsDataToStore;

	FSlateFontInfo TitleTextFont = FCoreStyle::Get().GetFontStyle(FName("EmbossedText"));

	TitleTextFont.Size = 30;

	FSlateFontInfo InteractButtonFont = FCoreStyle::Get().GetFontStyle(FName("EmbossedText"));

	InteractButtonFont.Size = 18;

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
			.VAlign(VAlign_Fill)
			[
				SNew(SScrollBox)
				+ SScrollBox::Slot()
				[
					ConsructAssetListView()
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBox)
				.HeightOverride(50.f)
				[
					SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.Padding(5.f)
						.VAlign(VAlign_Fill)
						.HAlign(HAlign_Fill)
						[
							SNew(SButton)
							.OnClicked(this, &SAdvanceDeletionTab::OnDeleteAllButtonClicked)
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Center)
							[
								SNew(STextBlock)
								.Text(FText::FromString(TEXT("Delete All")))
								.Font(InteractButtonFont)
							]
						]
						+ SHorizontalBox::Slot()
						.Padding(5.f)
						.VAlign(VAlign_Fill)
						.HAlign(HAlign_Fill)
						[
							SNew(SButton)
							.OnClicked(this, &SAdvanceDeletionTab::OnSelectAllButtonClicked)
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Center)
							[
								SNew(STextBlock)
								.Text(FText::FromString(TEXT("Delete All")))
							.Font(InteractButtonFont)
							]
						]
						+ SHorizontalBox::Slot()
						.Padding(5.f)
						.VAlign(VAlign_Fill)
						.HAlign(HAlign_Fill)
						[
							SNew(SButton)
							.OnClicked(this, &SAdvanceDeletionTab::OnDeSelectAllButtonClicked)
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Center)
							[
								SNew(STextBlock)
								.Text(FText::FromString(TEXT("Delete All")))
							.Font(InteractButtonFont)
							]
						]
				]
			]
		];
}

TSharedRef<SListView<TSharedPtr<FAssetData>>> SAdvanceDeletionTab::ConsructAssetListView()
{
	ConstructedAssetListView =  SNew(SListView<TSharedPtr<FAssetData>>)
		.ItemHeight(24.f)
		.ListItemsSource(&StoreAssetsDataUnderSelectedFolderArray)
		.OnGenerateRow(this, &SAdvanceDeletionTab::OnGenerateRowForList);

	return ConstructedAssetListView.ToSharedRef();
}

FReply SAdvanceDeletionTab::OnDeleteButtonClicked(TSharedPtr<FAssetData> ClickedAssetData)
{
	//DebugHeader::Print(ClickedAssetData->AssetName.ToString() + TEXT(" is clicked"),FColor::Green);
	FSuperManagerModule& SuperManagerModule = FModuleManager::LoadModuleChecked<FSuperManagerModule>(TEXT("SuperManager"));

	if (SuperManagerModule.DeleteSingleAssetForAssetList(*ClickedAssetData.Get()))
	{
		//Updating the list source items
		if (StoreAssetsDataUnderSelectedFolderArray.Contains(ClickedAssetData)) 
		{
			StoreAssetsDataUnderSelectedFolderArray.Remove(ClickedAssetData);
		}
		//Refresh the List;
		RefreshAssetListView();
	}
	return FReply::Handled();
}

FReply SAdvanceDeletionTab::OnDeleteAllButtonClicked()
{
	FSuperManagerModule& SuperManagerModule = FModuleManager::LoadModuleChecked<FSuperManagerModule>(TEXT("SuperManager"));

	TArray<FAssetData> AssetsToDelete;

	int32 DeletedAssetNum = StoreAssetsDataWillBeDeletedArray.Num();

	for(int i = 0;i<StoreAssetsDataWillBeDeletedArray.Num();i++)
	{
		AssetsToDelete.Add(*StoreAssetsDataWillBeDeletedArray[i].Get());
		if (StoreAssetsDataUnderSelectedFolderArray.Contains(StoreAssetsDataWillBeDeletedArray[i])) {
			StoreAssetsDataUnderSelectedFolderArray.Remove(StoreAssetsDataWillBeDeletedArray[i]);
		}
	}

	if (SuperManagerModule.DeleteAllAssetForAssetList(AssetsToDelete))
	{
		DebugHeader::ShowNotifyInfo(TEXT("Delete Selected "+FString::FromInt(DeletedAssetNum)+" Asset Successfully!"));
		StoreAssetsDataWillBeDeletedArray.Empty();
		RefreshAssetListView();
	}
	return FReply::Handled();
}

FReply SAdvanceDeletionTab::OnSelectAllButtonClicked()
{
	for(TSharedPtr<SCheckBox> CBox : StoreSCheckBoxArray)
	{
		CBox->SetIsChecked(ECheckBoxState::Checked);
	}
	StoreAssetsDataWillBeDeletedArray = StoreAssetsDataUnderSelectedFolderArray;
	return FReply::Handled();
}

FReply SAdvanceDeletionTab::OnDeSelectAllButtonClicked()
{
	for (TSharedPtr<SCheckBox> CBox : StoreSCheckBoxArray)
	{
		CBox->SetIsChecked(ECheckBoxState::Unchecked);
	}
	StoreAssetsDataWillBeDeletedArray.Empty();
	return FReply::Handled();
}

void SAdvanceDeletionTab::OnCheckBoxStateChanged(ECheckBoxState NewState, TSharedPtr<FAssetData> AssetData)
{
	switch (NewState)
	{
	case ECheckBoxState::Unchecked:
		if (StoreAssetsDataWillBeDeletedArray.Contains(AssetData)) StoreAssetsDataWillBeDeletedArray.Remove(AssetData);
		break;
	case ECheckBoxState::Checked:
		if (!StoreAssetsDataWillBeDeletedArray.Contains(AssetData)) StoreAssetsDataWillBeDeletedArray.Add(AssetData);
		break;
	default:
		break;
	}
}

#pragma region RowWidgetForAssetListView
TSharedRef<ITableRow> SAdvanceDeletionTab::OnGenerateRowForList(TSharedPtr<FAssetData> AssetDataToDisplay, const TSharedRef<STableViewBase>& OwnerTable)
{

	if (!AssetDataToDisplay.IsValid()) return SNew(STableRow<TSharedPtr<FAssetData>>, OwnerTable);

	const FString DisplayAssetName = AssetDataToDisplay->AssetName.ToString();

	const FString DisplayAssetClassName = AssetDataToDisplay->AssetClassPath.ToString();

	FSlateFontInfo AssetClassNameFont = GetEmboseedTextFont();
	AssetClassNameFont.Size = 10;

	FSlateFontInfo AssetNameFont = GetEmboseedTextFont();
	AssetNameFont.Size = 15;

	TSharedRef<STableRow<TSharedPtr<FAssetData>>> ListViewRowWidget = SNew(STableRow<TSharedPtr<FAssetData>>, OwnerTable)
		.Padding(FMargin(5.f))
		[
			SNew(SHorizontalBox)
			//First slot for checkbox
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.FillWidth(.05f)
		[
			ConstructCheckBox(AssetDataToDisplay)
		]

	//Second slot for displaying asset class name
	+ SHorizontalBox::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		[
			ConstructTextForRowWidget(DisplayAssetClassName, AssetClassNameFont)
		]
	//Third slot for displaying asset name
	+ SHorizontalBox::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			ConstructTextForRowWidget(DisplayAssetName, AssetNameFont)
		]
	//Fourth slot for a button
	+ SHorizontalBox::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Fill)
		[
			ConstructButtonForRowWidget(AssetDataToDisplay)
		]
		];

	return ListViewRowWidget;
}

TSharedRef<SCheckBox> SAdvanceDeletionTab::ConstructCheckBox(const TSharedPtr<FAssetData>& AssetDataToDisplay)
{
	TSharedRef<SCheckBox> ConstructedCheckBox =
		SNew(SCheckBox)
		.Type(ESlateCheckBoxType::CheckBox)
		.OnCheckStateChanged(this, &SAdvanceDeletionTab::OnCheckBoxStateChanged, AssetDataToDisplay)
		.Visibility(EVisibility::Visible);

	StoreSCheckBoxArray.Add(ConstructedCheckBox);
	
	return ConstructedCheckBox;
}

TSharedRef<SButton> SAdvanceDeletionTab::ConstructButtonForRowWidget(const TSharedPtr<FAssetData>& AssetDataToDisplay)
{
	TSharedRef<SButton> ConstructButton = SNew(SButton)
		.Text(FText::FromString("Delete"))
		.OnClicked(this, &SAdvanceDeletionTab::OnDeleteButtonClicked, AssetDataToDisplay);

	return ConstructButton;
}

TSharedRef<STextBlock> SAdvanceDeletionTab::ConstructTextForRowWidget(const FString& TextContent, const FSlateFontInfo& FontToUse)
{
	TSharedRef<STextBlock> ConstructedTextBlock = SNew(STextBlock)
		.Text(FText::FromString(TextContent))
		.Font(FontToUse)
		.ColorAndOpacity(FColor::White);

	return ConstructedTextBlock;
}
#pragma endregion

void SAdvanceDeletionTab::RefreshAssetListView()
{
	if(ConstructedAssetListView.IsValid())
	{
		ConstructedAssetListView->RebuildList();
	}
}
