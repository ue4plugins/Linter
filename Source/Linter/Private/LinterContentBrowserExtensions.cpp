// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "LinterContentBrowserExtensions.h"
#include "ContentBrowserMenuContexts.h"
#include "LinterStyle.h"
#include "ContentBrowserModule.h"
#include "Linter.h"
#include "BatchRenameTool/BatchRenameTool.h"
#include "TooltipEditor/TooltipTool.h"
#include "Misc/EngineVersionComparison.h"


#define LOCTEXT_NAMESPACE "Linter"

DEFINE_LOG_CATEGORY_STATIC(LinterContentBrowserExtensions, Log, All);


namespace {

void RunLinterForAssets(const TArray<FString>& SelectedPaths) {
    // Set Paths to Linter
    if (FLinterModule* Linter = FModuleManager::GetModulePtr<FLinterModule>("Linter")) {
        Linter->SetDesiredLintPaths(SelectedPaths);
    }

    // Execute Linter
#if UE_VERSION_NEWER_THAN(4, 26, 0)
    FGlobalTabmanager::Get()->TryInvokeTab(FName("LinterTab"));
#else
    FGlobalTabmanager::Get()->InvokeTab(FName("LinterTab"));
#endif
}

void EditBlueprintTooltips(const TArray<FAssetData> SelectedAssets) {
    UE_LOG(LinterContentBrowserExtensions, Display, TEXT("Opening Tooltip Tool window."));
    FTooltipTool{SelectedAssets}.ShowModal();
}

void BatchRenameAssets(const TArray<FAssetData> SelectedAssets) {
    UE_LOG(LinterContentBrowserExtensions, Display, TEXT("Starting batch rename."));
    FDlgBatchRenameTool{SelectedAssets}.ShowModal();
}

// Asset Context Menu is dynamic -> some options will only be displayed if
// you have certain types of Blueprints or Assets selected
void CreateDynamicAssetSelectionMenu(UToolMenu* InMenu) {
    const auto* Context = InMenu->FindContext<UContentBrowserDataMenuContext_FileMenu>();
    FToolMenuSection& Section = InMenu->AddSection("Linter", LOCTEXT("LinterSection", "Linter"));

    // Convert BrowserItems to their AssetData
    TArray<FAssetData> Assets;
    for (const auto& Asset : Context->SelectedItems) {
        FAssetData AssetData;
        Asset.Legacy_TryGetAssetData(AssetData);
        Assets.Add(AssetData);
    }
    
    // Run through the assets to determine if any are blueprints or can be renamed
    bool bAnyBlueprintsSelected = false;
    bool bAnyAssetCanBeRenamed = false;
    
    for (const auto& Asset : Assets) {
        // Cannot rename redirectors or classes or cooked packages
#if UE_VERSION_NEWER_THAN(5, 1, 0)
        if (!Asset.IsRedirector() && Asset.AssetClassPath.GetAssetName() != NAME_Class && !(Asset.PackageFlags & PKG_FilterEditorOnly))
#else
        if (!Asset.IsRedirector() && Asset.AssetClass != NAME_Class && !(Asset.PackageFlags & PKG_FilterEditorOnly))
#endif
        {
            bAnyAssetCanBeRenamed = true;
            
            if (Asset.GetClass()->IsChildOf(UBlueprint::StaticClass())) {
                bAnyBlueprintsSelected = true;
                break;
            }
        }
    }

    // If we have blueprints selected, add Tooltip Editor
    if (bAnyBlueprintsSelected) {
        Section.AddMenuEntry(
            "EditBlueprintTooltips",
            LOCTEXT("EditBlueprintTooltips", "Edit Blueprint Tooltips (Experimental)"),
            LOCTEXT("EditBlueprintTooltips_Tooltip", "Edit selected blueprints' templates definitions"),
            FSlateIcon(FLinterStyle::GetStyleSetName(), "Linter.Toolbar.Icon"),
            FExecuteAction::CreateStatic(&EditBlueprintTooltips, Assets)
        );
    }

    // If blueprints can be renamed, add the batch rename option
    if (bAnyAssetCanBeRenamed) {
        Section.AddMenuEntry(
            "BatchRename",
            LOCTEXT("BatchRenameAssets", "Batch Rename Assets (Experimental)"),
            LOCTEXT("BatchRenameAssets_Tooltip", "Perform a bulk rename operation on all of the selected assets"),
            FSlateIcon(FLinterStyle::GetStyleSetName(), "Linter.Toolbar.Icon"),
            FExecuteAction::CreateStatic(&BatchRenameAssets, Assets)
        );
    }
}

}


void FLinterContentBrowserExtensions::InstallHooks() {
    // Run Linter on Selected Folder(s)
    UToolMenu* ContentBrowserMenu = UToolMenus::Get()->ExtendMenu("ContentBrowser.FolderContextMenu");
    ContentBrowserMenu->AddSection("Linter", LOCTEXT("LinterSection", "Linter"))
    .AddMenuEntry(
        "LintAssets",
        LOCTEXT("ScanWithLinter", "Scan with Linter"),
        LOCTEXT("ScanWithLinter_Tooltip", "Scan project content with Linter"),
        FSlateIcon(FLinterStyle::GetStyleSetName(), "Linter.Toolbar.Icon"),
        FToolMenuExecuteAction::CreateLambda([](const FToolMenuContext& InContext) {
            if (const auto* Context = InContext.FindContext<UContentBrowserFolderContext>()) {
                RunLinterForAssets(Context->SelectedPackagePaths);
            }
        })
    );

    // BatchRename and TooltipEditor
    UToolMenu* AssetMenu = UToolMenus::Get()->ExtendMenu("ContentBrowser.AssetContextMenu");
    AssetMenu->AddDynamicSection("Linter", FNewToolMenuDelegate::CreateStatic(&CreateDynamicAssetSelectionMenu), {"AssetContextExploreMenuOptions", EToolMenuInsertType::After});
}

void FLinterContentBrowserExtensions::RemoveHooks() {
    UToolMenu* ContentBrowserMenu = UToolMenus::Get()->ExtendMenu("ContentBrowser.FolderContextMenu");
    ContentBrowserMenu->RemoveSection("Linter");

    UToolMenu* AssetMenu = UToolMenus::Get()->ExtendMenu("ContentBrowser.AssetContextMenu");
    AssetMenu->RemoveSection("Linter");
}

#undef LOCTEXT_NAMESPACE
