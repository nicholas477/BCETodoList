// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FToolBarBuilder;
class FMenuBuilder;
class UTodoListAsset;

class FBCETodoListModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	/** This function will be bound to Command (by default it will bring up plugin window) */
	void PluginButtonClicked();
	
	const TArray<UTodoListAsset*>& GetTodoLists() const { return TodoLists; };

	static inline FBCETodoListModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FBCETodoListModule>("BCETodoList");
	}

protected:

	void RegisterMenus();

	TSharedRef<class SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs);

	TSharedPtr<class FUICommandList> PluginCommands;
	TArray<UTodoListAsset*> TodoLists;

	TSharedPtr<class FTodoListAssetTypeActions> TodoListAssetTypeActions;

	void OnAssetManagerCreated();
	void OnAssetAdded(const FAssetData& AssetData);
	void OnAssetRemoved(const FAssetData& AssetData);
	void OnAssetCreated(UObject* Object);
};
