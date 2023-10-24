// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SAdvanceDeletionTab :public SCompoundWidget {
	SLATE_BEGIN_ARGS(SAdvanceDeletionTab) {}

	SLATE_ARGUMENT(TArray<TSharedPtr<FAssetData>>,AssetsDataToStore)

	SLATE_END_ARGS()
public:
	void Construct(const FArguments& InArgs);

private:

	TArray<TSharedPtr<FAssetData>> StoreAssetsDataUnderSelectedFolderArray;

	TArray<TSharedPtr<FAssetData>> StoreAssetsDataWillBeDeletedArray;

	TArray<TSharedRef<SCheckBox>> StoreSCheckBoxArray;

	TSharedRef<SListView<TSharedPtr<FAssetData>>> ConsructAssetListView();

	TSharedPtr<SListView<TSharedPtr<FAssetData>>> ConstructedAssetListView;
  
	FReply OnDeleteButtonClicked(TSharedPtr<FAssetData> ClickedAssetData);

	FReply OnDeleteAllButtonClicked();

	FReply OnSelectAllButtonClicked();

	FReply OnDeSelectAllButtonClicked();
	
	void OnCheckBoxStateChanged(ECheckBoxState NewState, TSharedPtr<FAssetData> AssetData);

	void RefreshAssetListView();
#pragma region RowWidgetForAssetListView
	TSharedRef<ITableRow> OnGenerateRowForList(TSharedPtr<FAssetData> AssetDataToDisplay,
		const TSharedRef<STableViewBase>& OwnerTable);

	TSharedRef<SCheckBox> ConstructCheckBox(const TSharedPtr<FAssetData>& AssetDataToDisplay);

	TSharedRef<SButton> ConstructButtonForRowWidget(const TSharedPtr<FAssetData>& AssetDataToDisplay);

	TSharedRef<STextBlock> ConstructTextForRowWidget(const FString& TextContent, const FSlateFontInfo& FontToUse);

#pragma endregion
	FSlateFontInfo GetEmboseedTextFont() const { return FCoreStyle::Get().GetFontStyle(FName("EmbossedText")); };
};



