// Copyright 2020 Gamemakin LLC. All Rights Reserved.

#include "LinterCommandlet.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "Linter.h"
#include "LinterSettings.h"
#include "LintRule.h"
#include "LintRuleSet.h"

DEFINE_LOG_CATEGORY_STATIC(LinterCommandlet, All, All);

ULinterCommandlet::ULinterCommandlet(const FObjectInitializer& ObjectInitializer) :
    Super(ObjectInitializer) {
    IsClient = false;
    IsServer = false;
}

static void PrintUsage() {
    UE_LOG(LinterCommandlet, Display, TEXT("Linter Usage: {Editor}.exe Project.uproject -run=Linter \"/Game/\""));
    UE_LOG(LinterCommandlet, Display, TEXT(""));
    UE_LOG(LinterCommandlet, Display, TEXT("This will run the Linter on the provided project and will scan the supplied directory, example being the project's full Content/Game tree. Can add multiple paths as additional arguments."));
}

int32 ULinterCommandlet::Main(const FString& InParams) {
    FString Params = InParams;
    // Parse command line.
    TArray<FString> Paths;
    TArray<FString> Switches;
    TMap<FString, FString> ParamsMap;
    ParseCommandLine(*Params, Paths, Switches, ParamsMap);

    UE_LOG(LinterCommandlet, Display, TEXT("Linter is indeed running!"));
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

    UE_LOG(LinterCommandlet, Display, TEXT("Loading the asset registry..."));
    AssetRegistryModule.Get().SearchAllAssets(/*bSynchronousSearch =*/true);
    UE_LOG(LinterCommandlet, Display, TEXT("Finished loading the asset registry. Determining Rule Set..."));

    ULintRuleSet* RuleSet = GetDefault<ULinterSettings>()->DefaultLintRuleSet.LoadSynchronous();
    if (ParamsMap.Contains(TEXT("RuleSet"))) {
        const FString RuleSetName = *ParamsMap.FindChecked(TEXT("RuleSet"));
        UE_LOG(LinterCommandlet, Display, TEXT("Trying to find Rule Set with Commandlet Name: %s"), *RuleSetName);

        FLinterModule::TryToLoadAllLintRuleSets();

        TArray<FAssetData> FoundRuleSets;
#if UE_VERSION_NEWER_THAN(5, 1, 0)
        AssetRegistryModule.Get().GetAssetsByClass(ULintRuleSet::StaticClass()->GetClassPathName(), FoundRuleSets, true);
#else
        AssetRegistryModule.Get().GetAssetsByClass(ULintRuleSet::StaticClass()->GetFName(), FoundRuleSets, true);
#endif

        for (const FAssetData& RuleSetData : FoundRuleSets) {
            ULintRuleSet* LoadedRuleSet = Cast<ULintRuleSet>(RuleSetData.GetAsset());
            if (LoadedRuleSet != nullptr && LoadedRuleSet->NameForCommandlet == RuleSetName) {
                RuleSet = LoadedRuleSet;
                UE_LOG(LinterCommandlet, Display, TEXT("Found Rule Set for name %s: %s"), *RuleSetName, *RuleSet->GetFullName());
            }
        }
    } else {
        UE_LOG(LinterCommandlet, Display, TEXT("Using default rule set..."));
    }

    if (RuleSet == nullptr) {
        UE_LOG(LinterCommandlet, Error, TEXT("Failed to load a rule set. Aborting. Returning error code 1."));
        return 1;
    }
    UE_LOG(LinterCommandlet, Display, TEXT("Using rule set: %s"), *RuleSet->GetFullName());

    if (Paths.Num() == 0) {
        Paths.Add(TEXT("/Game"));
    }
    UE_LOG(LinterCommandlet, Display, TEXT("Attempting to Lint paths: %s"), *FString::Join(Paths, TEXT(", ")));

    const ULintResults* LintResults = RuleSet->LintPath(Paths);
    UE_LOG(LinterCommandlet, Display, TEXT("%s"), *LintResults->Result.ToString());

    if (Switches.Contains("json") || ParamsMap.Contains("json") || Switches.Contains("html") || ParamsMap.Contains("html")) {
        UE_LOG(LinterCommandlet, Display, TEXT("Generating output report..."));

        // Write JSON file if requested
        if (Switches.Contains("json") || ParamsMap.Contains(FString("json"))) {
            FDateTime Now = FDateTime::Now();
            FString JsonOutputName = "lint-report-" + Now.ToString() + ".json";

            const FString LintReportPath = FPaths::ProjectSavedDir() / "LintReports";
            FString FullOutputPath = LintReportPath / JsonOutputName;

            if (ParamsMap.Contains("json")) {
                const FString JsonOutputOverride = *ParamsMap.FindChecked(FString("json"));
                if (FPaths::IsRelative(JsonOutputOverride)) {
                    FullOutputPath = LintReportPath / JsonOutputOverride;
                } else {
                    FullOutputPath = JsonOutputOverride;
                }
            }

            FullOutputPath = FPaths::ConvertRelativePathToFull(FullOutputPath);
            IFileManager::Get().MakeDirectory(*FPaths::GetPath(FullOutputPath), true);
            UE_LOG(LinterCommandlet, Display, TEXT("Exporting JSON report to %s"), *FullOutputPath);
            
            const FString ReportString = LintResults->GenerateJsonReportString();
            if (FFileHelper::SaveStringToFile(ReportString, *FullOutputPath)) {
                UE_LOG(LinterCommandlet, Display, TEXT("Exported JSON report successfully."));
            } else {
                UE_LOG(LinterCommandlet, Error, TEXT("Failed to export JSON report. Aborting. Returning error code 1."));
                return 1;
            }
        }

        // write HTML report if requested
        if (Switches.Contains(TEXT("html")) || ParamsMap.Contains(FString(TEXT("html")))) {
            FDateTime Now = FDateTime::Now();
            FString HtmlOutputName = TEXT("lint-report-") + Now.ToString() + TEXT(".html");

            const FString LintReportPath = FPaths::ProjectSavedDir() / TEXT("LintReports");
            FString FullOutputPath = LintReportPath / HtmlOutputName;

            if (ParamsMap.Contains(FString(TEXT("html")))) {
                const FString HtmlOutputOverride = *ParamsMap.FindChecked(TEXT("html"));
                if (FPaths::IsRelative(HtmlOutputName)) {
                    HtmlOutputName = HtmlOutputOverride;
                    FullOutputPath = LintReportPath / HtmlOutputName;
                } else {
                    FullOutputPath = HtmlOutputOverride;
                }
            }

            FullOutputPath = FPaths::ConvertRelativePathToFull(FullOutputPath);
            IFileManager::Get().MakeDirectory(*FPaths::GetPath(FullOutputPath), true);
            UE_LOG(LinterCommandlet, Display, TEXT("Exporting HTML report to %s"), *FullOutputPath);

            const FString HTMLReport = LintResults->GenerateHTML();
            if (FFileHelper::SaveStringToFile(HTMLReport, *FullOutputPath)) {
                UE_LOG(LinterCommandlet, Display, TEXT("Exported HTML report successfully."));
            } else {
                UE_LOG(LinterCommandlet, Error, TEXT("Failed to export HTML report. Aborting. Returning error code 1."));
                return 1;
            }
        }
    }

    if (LintResults->Errors > 0 || (Switches.Contains(TEXT("TreatWarningsAsErrors")) && LintResults->Warnings > 0)) {
        UE_LOG(LinterCommandlet, Display, TEXT("Lint completed with errors. Returning error code 2."));
        return 2;
    }

    return 0;
}
