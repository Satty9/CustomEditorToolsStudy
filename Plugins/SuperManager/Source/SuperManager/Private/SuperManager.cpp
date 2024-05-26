// Copyright Epic Games, Inc. All Rights Reserved.

#include "SuperManager.h"
#include "ContentBrowserModule.h"
#include "DebugHeader.h"
#include "EditorAssetLibrary.h"
#include "ObjectTools.h"
#include "AssetActions/QuickAssetAction.h"



#define LOCTEXT_NAMESPACE "FSuperManagerModule"



void FSuperManagerModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	InitContentBrowserMenuExtention();
	RegisterAdvanceDelitionTab();
}


#pragma region ContentBrowserMenuExtention
void FSuperManagerModule::InitContentBrowserMenuExtention()
{
	FContentBrowserModule& ContentBrowserModule =
	FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

	// Берём хранилище всех кнопок меню
	TArray<FContentBrowserMenuExtender_SelectedPaths>& ContentBrowserModuleMenuExtenders =
	ContentBrowserModule.GetAllPathViewContextMenuExtenders();

	// Добавляем нашу кнопку в хранилище кнопок
	//FContentBrowserMenuExtender_SelectedPaths CustomContentBrowserMenuDelegate;
	//CustomContentBrowserMenuDelegate.BindRaw(this, &FSuperManagerModule::CustomContentBrowserMenuExtender);
	//ContentBrowserModuleMenuExtenders.Add(CustomContentBrowserMenuDelegate);
	ContentBrowserModuleMenuExtenders.Add(FContentBrowserMenuExtender_SelectedPaths::CreateRaw(this, &FSuperManagerModule::CustomContentBrowserMenuExtender));
}


// Определяет положение меню в списке кнопок. Срабатывает при открытии редактора
TSharedRef<FExtender> FSuperManagerModule::CustomContentBrowserMenuExtender(const TArray<FString>& SelectedPaths)
{
	//TSharedRef<FExtender> MenuExtender(new FExtender());
	TSharedRef<FExtender> MenuExtender = MakeShareable(new FExtender());
	if(!SelectedPaths.IsEmpty())
	{
		MenuExtender->AddMenuExtension(FName("Delete"),
			EExtensionHook::After,
			TSharedPtr<FUICommandList>(), // Custom Hotkeys
			FMenuExtensionDelegate::CreateRaw(this, &FSuperManagerModule::AddContentBrowserMenuEntry)); // определяет информацию о кнопке

		FolderPathSelected = SelectedPaths;
	}
	return MenuExtender;
}


// Добавляет информацию о кнопке
void FSuperManagerModule::AddContentBrowserMenuEntry(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry(
		FText::FromString("Delete unused assets"),
		FText::FromString("Safely delete all unused assets under folder"),
		FSlateIcon(),
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnDeleteUnusedAssetButtonClicked)
		);

	MenuBuilder.AddMenuEntry(
		FText::FromString("Delete empty folders"),
		FText::FromString("Safely delete all empty folders"),
		FSlateIcon(),
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnDeleteEmptyFoldersButtonClicked)
		);
		
	MenuBuilder.AddMenuEntry(
    	FText::FromString("Advance deletion"),
    	FText::FromString("List asset by specific condition in a tab for deleting"),
    	FSlateIcon(),
    	FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnAdvanceDeletionButtonClicked)
    	);
}

#pragma region MenuExtentionFunctions
void FSuperManagerModule::OnDeleteUnusedAssetButtonClicked()
{
	if(FolderPathSelected.Num() > 1)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("You can do this only to one folder"));
		return;
	}
	
	TArray<FString> AssetPathNames = UEditorAssetLibrary::ListAssets(FolderPathSelected[0]);

	if(AssetPathNames.IsEmpty())
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No assets found under selected folder"), false);
		return;
	}

	const EAppReturnType::Type ConfirmResult = 
	DebugHeader::ShowMsgDialog(EAppMsgType::YesNo, TEXT("A total of ") + FString::FromInt(AssetPathNames.Num())
		+ TEXT(" found.\nWould you like to procceed?"), false);

	if(ConfirmResult == EAppReturnType::No) {return;}

	// Заранее чиним редиректоры, чтобы их не удалить в процессе дальнейшего удаления неиспользуемых ассетов
	UQuickAssetAction::FixUpRedirectors();
	
	TArray<FAssetData> UnusedAssetsDataArray;
	for(const FString& AssetPathName : AssetPathNames)
	{
		// Don`t touch root folder
		if(AssetPathName.Contains(TEXT("Developers")) ||
			AssetPathName.Contains(TEXT("Collections")) ||
			AssetPathName.Contains(TEXT("__ExternalActors__")) || // directory for artists
			AssetPathName.Contains(TEXT("__ExternalObjects__")))  // directory for artists
		{
			continue;
		}

		// Check is asset exist 
		if(!UEditorAssetLibrary::DoesAssetExist(AssetPathName)) {continue;}

		TArray<FString> AssetReferencers =
		UEditorAssetLibrary::FindPackageReferencersForAsset(AssetPathName);

		if(AssetReferencers.IsEmpty())
		{
			const FAssetData UnusedAssetData = UEditorAssetLibrary::FindAssetData(AssetPathName);
			UnusedAssetsDataArray.Add(UnusedAssetData);
		}
	}

	if(!UnusedAssetsDataArray.IsEmpty())
	{
		ObjectTools::DeleteAssets(UnusedAssetsDataArray);
	}
	else
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No unused asset found under selected folder"), false); 
	}
}

void FSuperManagerModule::OnDeleteEmptyFoldersButtonClicked()
{
	UQuickAssetAction::FixUpRedirectors();
	
	TArray<FString> FolderPathsArray = UEditorAssetLibrary::ListAssets(FolderPathSelected[0], true, true);
	uint32 DeletedFoldersCounter = 0;

	FString EmptyFolderPathName;
	TArray<FString> EmptyFoldersPathsNamesArray;

	for(const FString& FolderPathName : FolderPathsArray)
	{
		// Don`t touch root folder
		if(FolderPathName.Contains(TEXT("Developers")) ||
			FolderPathName.Contains(TEXT("Collections")) ||
			FolderPathName.Contains(TEXT("__ExternalActors__")) || // directory for artists
			FolderPathName.Contains(TEXT("__ExternalObjects__")))  // directory for artists
		{
			continue;
		}

		if (!UEditorAssetLibrary::DoesDirectoryExist(FolderPathName)) {continue;}

		if(!UEditorAssetLibrary::DoesDirectoryHaveAssets(FolderPathName))
		{
			EmptyFolderPathName.Append(FolderPathName);
			EmptyFolderPathName.Append(TEXT("\n"));

			EmptyFoldersPathsNamesArray.Add(FolderPathName);
		}
	}

	if(EmptyFoldersPathsNamesArray.IsEmpty())
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No empty folders found under selected folder"), false);
		return;
	}

	EAppReturnType::Type ConfirmResult = DebugHeader::ShowMsgDialog(EAppMsgType::OkCancel,
		TEXT("Empty folders found in:\n") + EmptyFolderPathName + TEXT("\nWould you like to delete all"), false);

	if(ConfirmResult==EAppReturnType::Cancel) {return;}

	for(const FString& EmptyFolderPath : EmptyFoldersPathsNamesArray)
	{
		UEditorAssetLibrary::DeleteDirectory(EmptyFolderPath) ? DeletedFoldersCounter++ : DebugHeader::Print(TEXT("Failed to delete " + EmptyFolderPathName), FColor::Red);
	}

	if(DeletedFoldersCounter > 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("Successfully deleted ") + FString::FromInt(DeletedFoldersCounter) + TEXT("folders"));
	}
}

void FSuperManagerModule::OnAdvanceDeletionButtonClicked()
{
	//DebugHeader::Print(TEXT("Working"), FColor::Green);
	FGlobalTabmanager::Get()->TryInvokeTab(FName("AdvanceDeletion"));
}
#pragma endregion 

#pragma endregion

#pragma region CustomEditorTab

void FSuperManagerModule::RegisterAdvanceDelitionTab()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(FName("AdvanceDeletion"),
		FOnSpawnTab::CreateRaw(this, &FSuperManagerModule::OnSpawnAdvanceDelitionTab))
		.SetDisplayName(FText::FromString(TEXT("Advance Delition")));
}

TSharedRef<SDockTab> FSuperManagerModule::OnSpawnAdvanceDelitionTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab).TabRole(NomadTab);
	
}

#pragma endregion

void FSuperManagerModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}



#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSuperManagerModule, SuperManager)