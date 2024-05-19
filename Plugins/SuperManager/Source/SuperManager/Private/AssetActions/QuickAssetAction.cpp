// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetActions/QuickAssetAction.h"
#include "DebugHeader.h"
#include "EditorUtilityLibrary.h"
#include "EditorAssetLibrary.h"
#include "ObjectTools.h"
#include "AssetRegistryModule.h"
#include "AssetToolsModule.h"

void UQuickAssetAction::DuplicateAssets(int32 NumOfDuplicates)
{
	if(NumOfDuplicates <= 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("Please, enter a VALID number"));
		return;
	}

	TArray<FAssetData> SelectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();
	uint32 Counter = 0;

	for(const FAssetData& SelectedAssetData : SelectedAssetsData)
	{
		for(int32 i = 0; i < NumOfDuplicates; i++)
		{ 
			// Deprecated
			// const FString SourceAssetPath = SelectedAssetData.ObjectPath.ToString();
			const FString SourceAssetPath = SelectedAssetData.GetSoftObjectPath().GetAssetPath().ToString();
			const FString NewDuplicatedAssetName = SelectedAssetData.AssetName.ToString() + TEXT("_") + FString::FromInt(i + 1);
			const FString NewPathName = FPaths::Combine(SelectedAssetData.PackagePath.ToString(), NewDuplicatedAssetName);

			if(UEditorAssetLibrary::DuplicateAsset(SourceAssetPath, NewPathName))
			{
				UEditorAssetLibrary::SaveAsset(NewPathName, false); // fasle = save always. And when asset dont change too
				++Counter;
			}
		}
	}

	if(Counter > 0)
	{
		/*Print(TEXT("Successfully duplicated" + FString::FromInt(Counter) + " files"), FColor::Green);*/
		DebugHeader::ShowNotifyInfo(TEXT("Successfully duplicated" + FString::FromInt(Counter) + " files"));
	}
	
}

void UQuickAssetAction::AddPrefix()
{
	TArray<UObject*> SelectedObjects = UEditorUtilityLibrary::GetSelectedAssets();
	uint32 Counter = 0;

	for(UObject* SelectedObject : SelectedObjects)
	{
		if(!SelectedObject) {continue;}

		const FString* FoundedPrefix = PrefixMap.Find(SelectedObject->GetClass());
		if(!FoundedPrefix || FoundedPrefix->IsEmpty())
		{
			DebugHeader::Print(TEXT("Failed to find prefix for class ") + SelectedObject->GetClass()->GetName(), FColor::Red);
			continue;
		}

		FString OldName = SelectedObject->GetName();

		if(OldName.StartsWith(*FoundedPrefix))
		{
			DebugHeader::Print(OldName + TEXT(" already has added prefix"), FColor::Red);
			continue;
		}

		if(SelectedObject->IsA(UMaterialInstanceConstant::StaticClass()))
		{
			OldName.RemoveFromStart("M_");
			OldName.RemoveFromEnd("_inst");
		}
		
		const FString NewNameWithPrefix = *FoundedPrefix + OldName;
		UEditorUtilityLibrary::RenameAsset(SelectedObject, NewNameWithPrefix);
		++Counter;
	}
	
	if(Counter > 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("Successfully renamed " + FString::FromInt(Counter) + " assets"));
	}
}

void UQuickAssetAction::RemoveUnusedAssets()
{
	TArray<FAssetData> SelectedAssets = UEditorUtilityLibrary::GetSelectedAssetData();
	TArray<FAssetData> UnusedAssetsData;

	FixUpRedirectors();
	
	for(const FAssetData& SelectedAssetData : SelectedAssets)
	{
		TArray <FString> AssetReferencer = 
		UEditorAssetLibrary::FindPackageReferencersForAsset(SelectedAssetData.GetObjectPathString());

		if(AssetReferencer.IsEmpty())
		{
			UnusedAssetsData.Add(SelectedAssetData);
		}
	}

	if(UnusedAssetsData.IsEmpty())
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No unused asset found among selected assets"),false);
		return;
	}

	const int32 NumOfAssetsDeleted = ObjectTools::DeleteAssets(UnusedAssetsData);
	if(NumOfAssetsDeleted == 0) {return;}

	DebugHeader::ShowNotifyInfo(TEXT("Successfully deleted " + FString::FromInt(NumOfAssetsDeleted) + TEXT(" unused assets")));
}


void UQuickAssetAction::FixUpRedirectors()
{
	TArray<UObjectRedirector*> RedirectrorsToFixArray;

	// загружаем модуль
	const FAssetRegistryModule& AssetRegistryModule =
	FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	// устанавливаем параметры фильтра - фильтр на редиректоры
	FARFilter Filter;
	Filter.bRecursivePaths = true;
	Filter.PackagePaths.Emplace("/Game");
	Filter.ClassNames.Emplace("ObjectRedirector");

	TArray<FAssetData> OutRedirectors;
	AssetRegistryModule.Get().GetAssets(Filter, OutRedirectors);

	for(const FAssetData& RedirectorData : OutRedirectors)
	{
		if (UObjectRedirector* RedirectorToFix = Cast<UObjectRedirector>(RedirectorData.GetAsset()))
		{
			RedirectrorsToFixArray.Add(RedirectorToFix);
		}
	}

	const FAssetToolsModule& AssetToolsModule =
	FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));

	AssetToolsModule.Get().FixupReferencers(RedirectrorsToFixArray);
}
