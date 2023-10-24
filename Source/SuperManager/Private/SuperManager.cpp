// Copyright Epic Games, Inc. All Rights Reserved.

#include "SuperManager.h"
#include "ContentBrowserModule.h"
#include "DebugHeader.h"
#include "EditorUtilityLibrary.h"
#include "EditorAssetLibrary.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "ObjectTools.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widget/AdvanceDeletionWidget.h"
#define LOCTEXT_NAMESPACE "FSuperManagerModule"

void FSuperManagerModule::StartupModule()
{
	InitCBMenuExtention();
	RegisterAdvanceDeletionTab();
}

void FSuperManagerModule::ShutdownModule()
{
	
}
#pragma region ContentBrowserMenuExtention
void FSuperManagerModule::InitCBMenuExtention()
{
	//加载模块
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	TArray<FContentBrowserMenuExtender_SelectedPaths>& ContentBrowserModuleExtenders = 
	ContentBrowserModule.GetAllPathViewContextMenuExtenders();

	FContentBrowserMenuExtender_SelectedPaths CustomCBMenuDelegate;
	//在鼠标右键调用出UI时，会出发执行CusCBMenuExtender函数
	CustomCBMenuDelegate.BindRaw(this,&FSuperManagerModule::CustomCBMenuExtender);
	
	ContentBrowserModuleExtenders.Add(CustomCBMenuDelegate);
}

TSharedRef<FExtender> FSuperManagerModule::CustomCBMenuExtender(const TArray<FString>& SelectedPaths)
{
	TSharedRef<FExtender> MenuExtender(new FExtender());

	if (SelectedPaths.Num() > 0) 
	{
		//按钮放置在Delete键的后面
		MenuExtender->AddMenuExtension(FName("Delete"),
			EExtensionHook::After,
			TSharedPtr<FUICommandList>(),//定制热键来触发
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
	MenuBuilder.AddMenuEntry(
		FText::FromString(TEXT("Delete Empty Folders")),
		FText::FromString(TEXT("Safely delete empty folders")),
		FSlateIcon(FAppStyle::Get().GetStyleSetName(), "LevelEditor.Tabs.Details"),
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnDeleteEmptyFoldersButtonClicked)
	);
	MenuBuilder.AddMenuEntry(
		FText::FromString(TEXT("Advance Deletion")),
		FText::FromString(TEXT("List assets by specific condition in a tab for deleting")),
		FSlateIcon(FAppStyle::Get().GetStyleSetName(), "LevelEditor.Tabs.Details"),
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnAdvanceDeletionButtonClicked)
	);
}

void FSuperManagerModule::OnDeleteUnusedAssetButtonClicked()
{
	//限制只能对一个文件夹进行操作
	if (FolderPathsSelected.Num() > 1) {
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok,("You can only do this to one folder"));
		return;
	}

	TArray<FString> AssetsPathNames = UEditorAssetLibrary::ListAssets(FolderPathsSelected[0]);

	if(AssetsPathNames.Num() == 0){
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No asset found under selected folder"));
		return;
	}
	EAppReturnType::Type ConfirmResult = DebugHeader::ShowMsgDialog(EAppMsgType::YesNo,TEXT("A total of ")+FString::FromInt(AssetsPathNames.Num()) + TEXT(" found.\nWoulde you like to procceed?"),false);

	if (ConfirmResult == EAppReturnType::No)return;

	FixUpRedirectors();

	TArray<FAssetData> UnusedAssetsDataArray;

	for (const FString& AssetPathName : AssetsPathNames){
		DebugHeader::Print(AssetPathName, FColor::Green);
		//如果所处理资产的路径名中包含Developers或者Collections，删除可能造成编辑器崩溃。
		if (AssetPathName.Contains(TEXT("Developers"))
			|| AssetPathName.Contains(TEXT("Collections"))
			|| AssetPathName.Contains(TEXT("__ExternalActors__"))
			|| AssetPathName.Contains(TEXT("__ExternalObjects__"))) {
			continue;
		}

		if (!UEditorAssetLibrary::DoesAssetExist(AssetPathName)) continue;
		//查询所删除资产是否被引用
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
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No unused asset found under selected folder"),false);
	}
}

void FSuperManagerModule::OnDeleteEmptyFoldersButtonClicked()
{
	FixUpRedirectors();

	TArray<FString> FolderPathsArray = UEditorAssetLibrary::ListAssets(FolderPathsSelected[0],true,true);
	uint32 Counter = 0;

	FString EmptyFolderPathsNames;
	TArray<FString> EmptyFoldersPathsArray;

	for (const FString& FolderPath:FolderPathsArray)
	{
		if (FolderPath.Contains(TEXT("Developers"))
			|| FolderPath.Contains(TEXT("Collections"))
			|| FolderPath.Contains(TEXT("__ExternalActors__"))
			|| FolderPath.Contains(TEXT("__ExternalObjects__"))) {
			continue;
		}
		if (!UEditorAssetLibrary::DoesDirectoryExist(FolderPath)) continue;

		if (!UEditorAssetLibrary::DoesDirectoryHaveAssets(FolderPath)) {
			EmptyFolderPathsNames.Append(FolderPath);
			EmptyFolderPathsNames.Append(TEXT("\n"));

			EmptyFoldersPathsArray.Add(FolderPath);
		}
	}
	if (EmptyFoldersPathsArray.Num() == 0) {
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No empty found under selected folder"),false);
		return;
	}
	EAppReturnType::Type ConfirmResult = 
	DebugHeader::ShowMsgDialog(EAppMsgType::OkCancel,TEXT("Empty folders found in:\n")+EmptyFolderPathsNames +TEXT("\nWould you like to delete all?"),false);

	if (ConfirmResult == EAppReturnType::Cancel) return;
	
	for (const FString& EmptyFolderPath : EmptyFoldersPathsArray) {
		UEditorAssetLibrary::DeleteDirectory(EmptyFolderPath) ?
			++Counter : DebugHeader::Print(TEXT("Failed to delete " + EmptyFolderPath),FColor::Red);
	}
	if(Counter > 0){
		DebugHeader::ShowNotifyInfo(TEXT("Successfully deleted ")+FString::FromInt(Counter)+TEXT(" folders."));
	}
}

void FSuperManagerModule::FixUpRedirectors()
{
	TArray<UObjectRedirector* > RedirectorToFixArray;
	FAssetRegistryModule& AssetRegistryModule =
		FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	FARFilter Filter;
	Filter.bRecursivePaths = true;//是否包括其子文件夹
	Filter.PackagePaths.Emplace("/Game");
	Filter.ClassNames.Emplace("ObjectRedirector");

	TArray<FAssetData> OutRedirectors;
	if (OutRedirectors.IsEmpty()) return;
	AssetRegistryModule.Get().GetAssets(Filter, OutRedirectors);
	for (const FAssetData& RedirectorData : OutRedirectors) {
		if (UObjectRedirector* RedirectorToFix = Cast<UObjectRedirector>(RedirectorData.GetAsset())) {
			RedirectorToFixArray.Add(RedirectorToFix);
		}
	}

	
	FAssetToolsModule& AssetToolsModule =
		FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));

	AssetToolsModule.Get().FixupReferencers(RedirectorToFixArray);
}

void FSuperManagerModule::OnAdvanceDeletionButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(FName("AdvanceDeletion"));
}
#pragma endregion

#pragma region CustomEditorTab
void FSuperManagerModule::RegisterAdvanceDeletionTab()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(FName("AdvanceDeletion"),
		FOnSpawnTab::CreateRaw(this, &FSuperManagerModule::OnSpawnAdvanceDeletionTab)
	).SetDisplayName(FText::FromString(TEXT("Advance Deletion")));
}

TSharedRef<SDockTab> FSuperManagerModule::OnSpawnAdvanceDeletionTab(const FSpawnTabArgs& SpawbTab)
{
	return SNew(SDockTab).TabRole(ETabRole::NomadTab)
		[
			SNew(SAdvanceDeletionTab)
			.AssetsDataToStore(GetAllAssetDataUnderSelectedFolder())
		];
}

TArray<TSharedPtr<FAssetData>> FSuperManagerModule::GetAllAssetDataUnderSelectedFolder()
{
	TArray<TSharedPtr<FAssetData>> AvaiableAssetData;


	if (FolderPathsSelected.IsEmpty()) {
		return AvaiableAssetData;
	}

	TArray<FString> AssetsPathNames = UEditorAssetLibrary::ListAssets(FolderPathsSelected[0]);

	for (const FString& AssetPathName : AssetsPathNames) {
		DebugHeader::Print(AssetPathName, FColor::Green);
		//如果所处理资产的路径名中包含Developers或者Collections，删除可能造成编辑器崩溃。
		if (AssetPathName.Contains(TEXT("Developers"))
			|| AssetPathName.Contains(TEXT("Collections"))
			|| AssetPathName.Contains(TEXT("__ExternalActors__"))
			|| AssetPathName.Contains(TEXT("__ExternalObjects__"))) {
			continue;
		}

		if (!UEditorAssetLibrary::DoesAssetExist(AssetPathName)) continue;

		const FAssetData Data = UEditorAssetLibrary::FindAssetData(AssetPathName);

		AvaiableAssetData.Add(MakeShared<FAssetData>(Data));
	}
	return AvaiableAssetData;
}



#pragma endregion

#pragma region ProccessDataForAdvanceDeletionTab
bool FSuperManagerModule::DeleteSingleAssetForAssetList(const FAssetData& AssetDataToDelete)
{
	TArray<FAssetData> AssetDataForDeletion;
	AssetDataForDeletion.Add(AssetDataToDelete);
	return ObjectTools::DeleteAssets(AssetDataForDeletion) == 0 ? false:true;
}

bool FSuperManagerModule::DeleteAllAssetForAssetList(const TArray<FAssetData>& AssetDataForDeletion)
{
	return ObjectTools::DeleteAssets(AssetDataForDeletion) == 0 ? false : true;
}
#pragma endregion
#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSuperManagerModule, SuperManager)