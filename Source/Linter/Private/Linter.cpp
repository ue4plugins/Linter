// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "Linter.h"
#include "ISettingsModule.h"
#include "Framework/Docking/TabManager.h"
#include "Styling/SlateStyle.h"
#include "PropertyEditorModule.h"
#include "LinterStyle.h"
#include "LinterContentBrowserExtensions.h"
#include "LinterNamingConvention.h"
#include "LinterSettings.h"
#include "UI/LintWizard.h"
#include "LintRuleSet.h"
#include "AssetRegistry/AssetRegistryModule.h"

#define LOCTEXT_NAMESPACE "FLinterModule"


static const FName LinterTabName = "LinterTab";


void FLinterModule::StartupModule() {
    // Load the asset registry module
    const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(FName("AssetRegistry"));
    IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

    if (AssetRegistry.IsLoadingAssets()) {
        AssetRegistry.OnFilesLoaded().AddRaw(this, &FLinterModule::OnInitialAssetRegistrySearchComplete);
    } else {
        OnInitialAssetRegistrySearchComplete();
    }

    // Integrate Linter actions into existing editor context menus
    if (!IsRunningCommandlet()) {
        // Register slate style overrides
        FLinterStyle::Initialize();
        const TSharedPtr<FSlateStyleSet> StyleSetPtr = FLinterStyle::StyleSet;

        if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings")) {
            SettingsModule->RegisterSettings("Project", "Plugins", "Linter",
                                             LOCTEXT("RuntimeSettingsName", "Linter"),
                                             LOCTEXT("RuntimeSettingsDescription", "Configure the Linter plugin"),
                                             GetMutableDefault<ULinterSettings>());
        }

        // Install UI Hooks
        FLinterContentBrowserExtensions::InstallHooks();

        //Register our UI
        FGlobalTabmanager::Get()->RegisterNomadTabSpawner(LinterTabName, FOnSpawnTab::CreateStatic(&FLinterModule::SpawnTab, StyleSetPtr))
                                .SetDisplayName(LOCTEXT("LinterTabName", "Linter"))
                                .SetTooltipText(LOCTEXT("LinterTabToolTip", "Linter"))
                                .SetMenuType(ETabSpawnerMenuType::Hidden);

        FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
        PropertyModule.RegisterCustomClassLayout(ULinterNamingConvention::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FLinterNamingConventionDetails::MakeInstance));
    }
}

void FLinterModule::ShutdownModule() {
    if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings")) {
        SettingsModule->UnregisterSettings("Project", "Plugins", "Linter");
    }

    if (UObjectInitialized()) {
        FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
        PropertyModule.UnregisterCustomClassLayout(ULinterNamingConvention::StaticClass()->GetFName());

        // Remove Hooks and Tabs
        FLinterContentBrowserExtensions::RemoveHooks();
        FGlobalTabmanager::Get()->UnregisterTabSpawner(LinterTabName);

        // Unregister slate style overrides
        FLinterStyle::Shutdown();
    }
}


TSharedRef<SDockTab> FLinterModule::SpawnTab(const FSpawnTabArgs& TabSpawnArgs, TSharedPtr<FSlateStyleSet> StyleSet) {
    const FSlateBrush* IconBrush = StyleSet->GetBrush("Linter.Toolbar.Icon");

    const TSharedRef<SDockTab> MajorTab =
        SNew(SDockTab)
        .TabRole(ETabRole::MajorTab);

    MajorTab->SetContent(SNew(SLintWizard));
    MajorTab->SetTabIcon(IconBrush);

    return MajorTab;
}

void FLinterModule::OnInitialAssetRegistrySearchComplete() {
    TryToLoadAllLintRuleSets();
}

void FLinterModule::TryToLoadAllLintRuleSets() {
    const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(FName("AssetRegistry"));
    const IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

    TArray<FAssetData> FoundRuleSets;
#if UE_VERSION_NEWER_THAN(5, 1, 0)
    AssetRegistry.GetAssetsByClass(ULintRuleSet::StaticClass()->GetClassPathName(), FoundRuleSets, true);
#else
    AssetRegistry.GetAssetsByClass(ULintRuleSet::StaticClass()->GetFName(), FoundRuleSets, true);
#endif

    // Attempt to get all RuleSets in memory so that linting tools are better aware of them
    for (const FAssetData& RuleSetData : FoundRuleSets) {
        if (!RuleSetData.IsAssetLoaded()) {
            (void)RuleSetData.GetAsset();
        }
    }
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FLinterModule, Linter)
DEFINE_LOG_CATEGORY(LogLinter);
