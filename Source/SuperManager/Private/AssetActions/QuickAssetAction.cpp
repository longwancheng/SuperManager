// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetActions/QuickAssetAction.h"
#include "DebugHeader.h"
#include "EditorUtilityLibrary.h"
#include "EditorAssetLibrary.h"
#include "ObjectTools.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
void UQuickAssetAction::DuplicateAsset(int32 NumOfDuplicates)
{
	if(NumOfDuplicates <= 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok,TEXT("Please enter a Valid number"),true);
		return;
	}

	TArray<FAssetData> SelectedAssetsData =  UEditorUtilityLibrary::GetSelectedAssetData();
	uint32 Counter = 0;

	for(const FAssetData& SelectedAssetData :SelectedAssetsData)
	{
		for(int32 i = 0;i<NumOfDuplicates;i++)
		{
			const FString SourceAssetPath = SelectedAssetData.ObjectPath.ToString();
			const FString NumDuplicatedAssetName = SelectedAssetData.AssetName.ToString() + TEXT("_")+FString::FromInt(i+1);
			const FString NewPathName = FPaths::Combine(SelectedAssetData.PackagePath.ToString(),NumDuplicatedAssetName);

			if (UEditorAssetLibrary::DuplicateAsset(SourceAssetPath,NewPathName))
			{
				UEditorAssetLibrary::SaveAsset(NewPathName,false);
				++Counter;
			}
		}
	}
	if(Counter>0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("Successfully duplicated "+FString::FromInt(Counter))+" files");
	}
}

void UQuickAssetAction::RemoveUnusedAsset()
{
	TArray<FAssetData> SelectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();
	TArray<FAssetData> UnusedAssetsData;

	FixUpRedirectors();

	for (const FAssetData& SelectedAssetData:SelectedAssetsData)
	{
		TArray<FString> AssetRefrencers = 
		UEditorAssetLibrary::FindPackageReferencersForAsset(SelectedAssetData.ObjectPath.ToString());
		if(AssetRefrencers.Num()==0)
		{
			UnusedAssetsData.Add(SelectedAssetData);
		}
	}
	if(UnusedAssetsData.Num() == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok,TEXT("No unused found among selected assets"),false);
		return;
	}

	int32 NumOfAssetDeleted = ObjectTools::DeleteAssets(UnusedAssetsData);
	if (NumOfAssetDeleted == 0) return;

	DebugHeader::ShowNotifyInfo(TEXT("Successfully deleted "+FString::FromInt(NumOfAssetDeleted))+"Assets");
}

void UQuickAssetAction::FixUpRedirectors()
{
	TArray<UObjectRedirector* > RedirectorToFixArray;
	FAssetRegistryModule& AssetRegistryModule = 
	FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	FARFilter Filter;
	Filter.bRecursivePaths = true;//是否包括其子文件夹
	Filter.PackagePaths.Emplace("/Game");
	Filter.ClassNames.Emplace("ObjectRedirector");
	TArray<FAssetData> OutRedirectors;
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

void UQuickAssetAction::AddPrefixs()
{
	TArray<UObject*> SelectedObjects = UEditorUtilityLibrary::GetSelectedAssets();
	uint32 Counter = 0;

	for (UObject* SelectedObejct:SelectedObjects)
	{
		if(!SelectedObejct) continue;
		FString* Prefixfound = PrefixMap.Find(SelectedObejct->GetClass());
		if(!Prefixfound||Prefixfound->IsEmpty())
		{
			DebugHeader::Print(TEXT("Failed to find prefix for class "+SelectedObejct->GetClass()->GetName()),FColor::Red);
			continue;
		}
		FString OldName = SelectedObejct->GetName();
		if(OldName.StartsWith(*Prefixfound))
		{
			DebugHeader::Print(OldName +TEXT(" already has prefix added"),FColor::Red);
			continue;
		}

		if(SelectedObejct ->IsA<UMaterialInstanceConstant>())
		{
			OldName.RemoveFromStart(TEXT("M_"));
			OldName.RemoveFromEnd(TEXT("_inst"));
		}
		const FString NewNameWithPrefix = *Prefixfound + OldName;

		UEditorUtilityLibrary::RenameAsset(SelectedObejct,NewNameWithPrefix);

		++Counter;
	}
	if(Counter>0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("Successfully renamed + "+FString::FromInt(Counter)+" assets"));
	}
}
