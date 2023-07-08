// Copyright Epic Games, Inc. All Rights Reserved.

#include "BCETodoList.h"
#include "BCETodoListStyle.h"
#include "BCETodoListCommands.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"
#include "TodoListAsset.h"
#include "Engine/AssetManager.h"
#include "TodoListAssetTypeActions.h"
#include "TodoListWidget.h"

static const FName BCETodoListTabName("BCETodoList");

#define LOCTEXT_NAMESPACE "FBCETodoListModule"

void FBCETodoListModule::StartupModule()
{
	UAssetManager::CallOrRegister_OnAssetManagerCreated(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FBCETodoListModule::OnAssetManagerCreated));

	FBCETodoListStyle::Initialize();
	FBCETodoListStyle::ReloadTextures();

	FBCETodoListCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FBCETodoListCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FBCETodoListModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FBCETodoListModule::RegisterMenus));
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(BCETodoListTabName, FOnSpawnTab::CreateRaw(this, &FBCETodoListModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FBCETodoListTabTitle", "Todo List"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);

	TodoListAssetTypeActions = MakeShared<FTodoListAssetTypeActions>();
	FAssetToolsModule::GetModule().Get().RegisterAssetTypeActions(TodoListAssetTypeActions.ToSharedRef());
}

void FBCETodoListModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FBCETodoListStyle::Shutdown();

	FBCETodoListCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(BCETodoListTabName);

	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		FAssetToolsModule::GetModule().Get().UnregisterAssetTypeActions(TodoListAssetTypeActions.ToSharedRef());
	}
}

TSharedRef<SDockTab> FBCETodoListModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	TSharedRef<STodoListWidget> DetailsView = SNew(STodoListWidget);

	UTodoListAsset* InitiallySelectedAsset = nullptr;

	FString AssetLongPackageName;
	GConfig->GetString(TEXT("BCETodoList"), TEXT("SelectedList"), AssetLongPackageName, GEditorIni);
	TSoftObjectPtr Asset(AssetLongPackageName);
	if (UObject* AssetPtr = Asset.LoadSynchronous())
	{
		InitiallySelectedAsset = Cast<UTodoListAsset>(AssetPtr);
	}

	if (InitiallySelectedAsset)
	{
		if (!TodoLists.Contains(InitiallySelectedAsset))
		{
			TodoLists.AddUnique(InitiallySelectedAsset);
		}
		DetailsView->SetTodoList(InitiallySelectedAsset);
	}

	return SNew(SDockTab)
	.TabRole(ETabRole::NomadTab)
	[
		// Put your tab content here!
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SComboBox<UTodoListAsset*>)
			.OptionsSource(&TodoLists)
			.InitiallySelectedItem(InitiallySelectedAsset)
			.OnGenerateWidget_Lambda([](UTodoListAsset* InAsset)
			{
				const FText NameText = InAsset ? InAsset->ListName : LOCTEXT("NoneName", "<None>");
				return SNew(STextBlock)
					.Text(NameText)
					.Font(IPropertyTypeCustomizationUtils::GetRegularFont());
			})
			.OnSelectionChanged_Lambda([DetailsView](UTodoListAsset* InAsset, ESelectInfo::Type)
			{
				// Save last opened level name.
				FString AssetLongPackageName = TSoftObjectPtr(InAsset).ToString();
				GConfig->SetString(TEXT("BCETodoList"), TEXT("SelectedList"), *AssetLongPackageName, GEditorIni);

				DetailsView->SetTodoList(InAsset);
			})
			.Content()
			[
				SNew(STextBlock)
				.Font(IPropertyTypeCustomizationUtils::GetRegularFont())
				.Text_Lambda([DetailsView]()
				{
					return DetailsView->GetTodoList() ? DetailsView->GetTodoList()->ListName : LOCTEXT("NoneName", "<None>");
				})
			]
		]
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			DetailsView
		]
	];
}

void FBCETodoListModule::OnAssetManagerCreated()
{
	UAssetManager& LocalAssetManager = UAssetManager::Get();
	IAssetRegistry& LocalAssetRegistry = LocalAssetManager.GetAssetRegistry();

	LocalAssetRegistry.OnAssetAdded().AddRaw(this, &FBCETodoListModule::OnAssetAdded);
	LocalAssetRegistry.OnAssetRemoved().AddRaw(this, &FBCETodoListModule::OnAssetRemoved);
	LocalAssetRegistry.OnInMemoryAssetCreated().AddRaw(this, &FBCETodoListModule::OnAssetCreated);
	//LocalAssetRegistry.OnAssetRenamed().AddRaw(this, &FBCETodoListModule::OnAssetRenamed);

	TArray<FAssetData> AssetData;
	LocalAssetRegistry.GetAssetsByClass(UTodoListAsset::StaticClass()->GetClassPathName(), AssetData);
	for (FAssetData& Asset : AssetData)
	{
		if (UTodoListAsset* TodoListAsset = Cast<UTodoListAsset>(Asset.GetAsset()))
		{
			TodoLists.AddUnique(TodoListAsset);
		}
	}
}

void FBCETodoListModule::OnAssetAdded(const FAssetData& AssetData)
{
	if (AssetData.AssetClassPath == UTodoListAsset::StaticClass()->GetClassPathName())
	{
		if (UTodoListAsset* TodoListAsset = Cast<UTodoListAsset>(AssetData.GetAsset()))
		{
			TodoLists.AddUnique(TodoListAsset);
		}
	}
}

void FBCETodoListModule::OnAssetRemoved(const FAssetData& AssetData)
{
	if (AssetData.AssetClassPath == UTodoListAsset::StaticClass()->GetClassPathName())
	{
		if (UTodoListAsset* TodoListAsset = Cast<UTodoListAsset>(AssetData.GetAsset()))
		{
			TodoLists.Remove(TodoListAsset);
		}
	}
}

void FBCETodoListModule::OnAssetCreated(UObject* Object)
{
	if (UTodoListAsset* TodoListAsset = Cast<UTodoListAsset>(Object))
	{
		TodoLists.AddUnique(TodoListAsset);
	}
}

void FBCETodoListModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(BCETodoListTabName);
}

void FBCETodoListModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddSeparator("TodoSeparator");
			Section.AddMenuEntryWithCommandList(FBCETodoListCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			Section.AddSeparator("TodoSeparator");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FBCETodoListCommands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FBCETodoListModule, BCETodoList)