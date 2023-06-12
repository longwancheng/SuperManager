// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetActions/QuickAssetAction.h"
#include "DebugHeader.h"
#include "EditorUtilityLibrary.h"
#include "EditorAssetLibrary.h"

void UQuickAssetAction::DuplicateAssets(int32 NumOfDupicates)
{
	if(NumOfDupicates<=0)
	{
		ShowMsgDialog(EAppMsgType::Ok,"Please enter a valid number");
		return;
	}
	TArray<FAssetData> SelectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();
	uint32 Counter = 0;

	for(const FAssetData& SelectedAssetData : SelectedAssetsData)
	{
		for(int32 i = 0;i<NumOfDupicates;i++)
		{
			const FString SourceAssetPath = SelectedAssetData.ObjectPath.ToString();
			const FString NewDuplicateedAssetName = SelectedAssetData.AssetName.ToString() + TEXT("_")+FString::FromInt(i+1);
			const FString NewPathName = FPaths::Combine(SelectedAssetData.PackagePath.ToString(),NewDuplicateedAssetName);
			if(UEditorAssetLibrary::DuplicateAsset(SourceAssetPath,NewPathName))
			{
					UEditorAssetLibrary::SaveAsset(NewPathName,false);
					++Counter;
			}
		}
	}
	if(Counter>0)
	{
		ShowNotifyInfo(TEXT("Successfully duplicated " + FString::FromInt(Counter)+" files"));
		//Print(TEXT("Successfully duplicated " + FString::FromInt(Counter)+" files"),FColor::Green);
	}
}
void UQuickAssetAction::AddPrefixs()
{
	TArray<UObject*> SelectedObjects = UEditorUtilityLibrary::GetSelectedAssets();
	uint32 Counter = 0;

	for(UObject* SelectedObject : SelectedObjects)
	{
		if(!SelectedObject) continue;
		FString* PrefixFound = PrefixMap.Find(SelectedObject->GetClass());

		if(!PrefixFound||PrefixFound->IsEmpty())
		{
			Print(TEXT("Failed to find prefix for class")+SelectedObject->GetClass()->GetName(),FColor::Red);
			continue;
		}
		FString OldName = SelectedObject->GetName();

		if(OldName.StartsWith(*PrefixFound))
		{
			Print(OldName+TEXT(" already has prefix added"),FColor::Red);
			continue;
		}
		FString NewNameWithPrefix = *PrefixFound + OldName;
		if(NewNameWithPrefix.Contains("M_") || NewNameWithPrefix.Contains("_inst"))
		{
			NewNameWithPrefix = NewNameWithPrefix.Replace(TEXT("M_"),TEXT(""));
			NewNameWithPrefix = NewNameWithPrefix.Replace(TEXT("_inst"),TEXT(""));
		};
		UEditorUtilityLibrary::RenameAsset(SelectedObject,NewNameWithPrefix );
		++Counter;
	}
	if(Counter>0)
	{
		ShowNotifyInfo(TEXT("Successfully renamed "+ FString::FromInt(Counter)+" assets"));
	}
}

