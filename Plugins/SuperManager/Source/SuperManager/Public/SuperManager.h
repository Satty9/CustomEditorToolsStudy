// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FSuperManagerModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:

#pragma region ContentBrowserMenuExtention
	void InitContentBrowserMenuExtention();

	TArray<FString> FolderPathSelected;
	TSharedRef<FExtender> CustomContentBrowserMenuExtender(const TArray<FString>& SelectedPaths);

	void AddContentBrowserMenuEntry(class FMenuBuilder& MenuBuilder);

#pragma region MenuExtentionFunctions
	
	void OnDeleteUnusedAssetButtonClicked();
	void OnDeleteEmptyFoldersButtonClicked();
	void OnAdvanceDeletionButtonClicked();
	
#pragma endregion
#pragma endregion 

#pragma region CustomEditorTab
	void RegisterAdvanceDelitionTab();

	TSharedRef<SDockTab> OnSpawnAdvanceDelitionTab(const FSpawnTabArgs& SpawnTabArgs);

	TArray<TSharedPtr <FAssetData> > GetAllAssetDataUnderSelectedFolder();
#pragma endregion
};
 