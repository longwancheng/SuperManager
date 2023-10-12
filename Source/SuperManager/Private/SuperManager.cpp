// Copyright Epic Games, Inc. All Rights Reserved.

#include "SuperManager.h"
#include "ContentBrowserModule.h"
#include "DebugHeader.h"
#include "EditorUtilityLibrary.h"
#include "EditorAssetLibrary.h"
#include "ObjectTools.h"
#define LOCTEXT_NAMESPACE "FSuperManagerModule"

void FSuperManagerModule::StartupModule()
{
	InitCBMenuExtention();
}

void FSuperManagerModule::ShutdownModule()
{

	
}
#pragma region ContentBrowserMenuExtention
void FSuperManagerModule::InitCBMenuExtention()
{
	//����ģ��
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	TArray<FContentBrowserMenuExtender_SelectedPaths>& ContentBrowserModuleExtenders = 
	ContentBrowserModule.GetAllPathViewContextMenuExtenders();

	FContentBrowserMenuExtender_SelectedPaths CustomCBMenuDelegate;
	//������Ҽ����ó�UIʱ�������ִ��CusCBMenuExtender����
	CustomCBMenuDelegate.BindRaw(this,&FSuperManagerModule::CustomCBMenuExtender);

	ContentBrowserModuleExtenders.Add(CustomCBMenuDelegate);
}

TSharedRef<FExtender> FSuperManagerModule::CustomCBMenuExtender(const TArray<FString>& SelectedPaths)
{
	TSharedRef<FExtender> MenuExtender(new FExtender());

	if (SelectedPaths.Num() > 0) 
	{
		//��ť������Delete���ĺ���
		MenuExtender->AddMenuExtension(FName("Delete"),
			EExtensionHook::After,
			TSharedPtr<FUICommandList>(),
			FMenuExtensionDelegate::CreateRaw(this,&FSuperManagerModule::AddCBMenuExtry)
			);
		FolderPathsSelected = SelectedPaths;
	}
	return MenuExtender;
}

void FSuperManagerModule::AddCBMenuExtry(class FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry(
		FText::FromString(TEXT("Delte Unused Assets")),
		FText::FromString(TEXT("Safely delete all unused assets under folders")),
		FSlateIcon(FAppStyle::Get().GetStyleSetName(), "LevelEditor.Tabs.Details"),
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnDeleteUnusedAssetButtonClicked)
	);
}

void FSuperManagerModule::OnDeleteUnusedAssetButtonClicked()
{
	//����ֻ�ܶ�һ���ļ��н��в���
	if (FolderPathsSelected.Num() > 1) {
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok,("You can only do this to one folder"));
		return;
	}

	TArray<FString> AssetsPathNames = UEditorAssetLibrary::ListAssets(FolderPathsSelected[0]);



	if(AssetsPathNames.Num() == 0){
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No asset found under selected folder"));
		return;
	}
	EAppReturnType::Type ConfirmResult = DebugHeader::ShowMsgDialog(EAppMsgType::YesNo,TEXT("A total of ")+FString::FromInt(AssetsPathNames.Num()) + TEXT(" found.\nWoulde you like to procceed?"));

	if (ConfirmResult == EAppReturnType::No)return;

	TArray<FAssetData> UnusedAssetsDataArray;

	for (const FString& AssetPathName : AssetsPathNames){
		DebugHeader::Print(AssetPathName, FColor::Green);
		if (AssetPathName.Contains(TEXT("Developers")) || AssetPathName.Contains(TEXT("Collections"))) {
			continue;
		}
		if (!UEditorAssetLibrary::DoesAssetExist(AssetPathName)) continue;
		UEditorAssetLibrary::FindPackageReferencersForAsset(AssetPathName);

		TArray<FString>AssetReferencers = UEditorAssetLibrary::FindPackageReferencersForAsset(AssetPathName);

		if (AssetReferencers.Num() == 0) {
			const FAssetData UnusedAssetData = UEditorAssetLibrary::FindAssetData(AssetPathName);
			UnusedAssetsDataArray.Add(UnusedAssetData);
		}
	}
	if (UnusedAssetsDataArray.Num() > 0) {
		ObjectTools::DeleteAssets(UnusedAssetsDataArray);
	}
	else {
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No unused asset found under selected folder"));
	}
}

#pragma endregion
#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSuperManagerModule, SuperManager)