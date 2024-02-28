#include "LintRuleSet.h"

#include "AnyObject_LinterDummyClass.h"
#include "IPluginManager.h"
#include "JsonObjectWrapper.h"
#include "LintRunner.h"
#include "Linter.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Modules/ModuleManager.h"
#include "HAL/RunnableThread.h"


TArray<TSharedPtr<FLintRuleViolation>> ULintResults::GetSharedViolations() const {
    TArray<TSharedPtr<FLintRuleViolation>> SharedRuleViolations;
    for (const FLintRuleViolation& Violation : Violations) {
        TSharedPtr<FLintRuleViolation> SharedViolation = MakeShared<FLintRuleViolation>(Violation);
        SharedViolation->PopulateAssetData();
        SharedRuleViolations.Push(SharedViolation);
    }

    return SharedRuleViolations;
}

TSharedPtr<FJsonObject> ULintResults::GenerateJsonReport() const {
    auto Report = MakeShared<FJsonObject>();

    Report->SetStringField("Project", FPaths::GetBaseFilename(FPaths::GetProjectFilePath()));
    Report->SetStringField("Result", Result.ToString());
    Report->SetNumberField("Warnings", Warnings);
    Report->SetNumberField("Errors", Errors);

    TArray<TSharedPtr<FJsonValue>> PathArray;
    for (const auto& Path : Paths) {
        PathArray.Add(MakeShared<FJsonValueString>(Path));
    }
    Report->SetArrayField("Paths", PathArray);

    TArray<TSharedPtr<FJsonValue>> AssetsArray;
    for (const auto& Asset : CheckedAssets) {
#if UE_VERSION_NEWER_THAN(5, 1, 0)
        AssetsArray.Add(MakeShared<FJsonValueString>(Asset.GetObjectPathString()));
#else
        AssetsArray.Add(MakeShared<FJsonValueString>(Asset.ObjectPath.ToString()));
#endif
    }
    Report->SetArrayField("CheckedAssets", AssetsArray);

    TArray<TSharedPtr<FJsonValue>> ViolationsArray;
    for (const UObject* Violator : FLintRuleViolation::AllRuleViolationViolators(Violations)) {
        TSharedPtr<FJsonObject> ViolationObject = MakeShareable(new FJsonObject);

        FAssetData AssetData;
        TArray<FLintRuleViolation> ViolatorViolations = FLintRuleViolation::AllRuleViolationsWithViolator(Violations, Violator);

        if (ViolatorViolations.Num() > 0) {
            ViolatorViolations[0].PopulateAssetData();
            AssetData = ViolatorViolations[0].ViolatorAssetData;

            ViolationObject->SetStringField("AssetName", AssetData.AssetName.ToString());
            ViolationObject->SetStringField("AssetFullName", AssetData.GetFullName());
#if UE_VERSION_NEWER_THAN(5, 1, 0)
            ViolationObject->SetStringField("AssetPath", AssetData.GetObjectPathString());
#else
            ViolationObject->SetStringField("ViolatorAssetPath", AssetData.ObjectPath.ToString());
#endif
            //@TODO: Thumbnail export?

            TArray<TSharedPtr<FJsonValue>> ViolationObjects;
            for (const FLintRuleViolation& Violation : ViolatorViolations) {
                ULintRule* LintRule = Violation.ViolatedRule->GetDefaultObject<ULintRule>();
                check(LintRule != nullptr);

                TSharedPtr<FJsonObject> RuleJsonObject = MakeShareable(new FJsonObject);
                RuleJsonObject->SetStringField("Group", LintRule->RuleGroup.ToString());
                RuleJsonObject->SetStringField("Title", LintRule->RuleTitle.ToString());
                RuleJsonObject->SetStringField("Description", LintRule->RuleDescription.ToString());
                RuleJsonObject->SetStringField("RuleURL", LintRule->RuleURL);
                RuleJsonObject->SetNumberField("Severity", static_cast<int32>(LintRule->RuleSeverity));
                RuleJsonObject->SetStringField("RecommendedAction", Violation.RecommendedAction.ToString());

                ViolationObjects.Add(MakeShared<FJsonValueObject>(RuleJsonObject));
            }

            ViolationObject->SetArrayField("Violations", ViolationObjects);
        }

        ViolationsArray.Add(MakeShared<FJsonValueObject>(ViolationObject));
    }
    Report->SetArrayField("Violators", ViolationsArray);
    
    return Report;
}

FString ULintResults::GenerateJsonReportString() const {
    const TSharedPtr<FJsonObject> Report = GenerateJsonReport();

    FString ReportString;
    const TSharedRef<TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>> Writer = TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&ReportString);
    FJsonSerializer::Serialize(Report.ToSharedRef(), Writer);

    return ReportString;
}

FString ULintResults::GenerateHTML() const {
    const FString ReportString = GenerateJsonReportString();
    
    static const FString TemplatePath = IPluginManager::Get().FindPlugin("Linter")->GetBaseDir() / "Resources" /"LintReportTemplate.html";
    UE_LOG(LogLinter, Display, TEXT("Loading HTML report template from %s"), *TemplatePath);
    
    FString Template;
    if (!FFileHelper::LoadFileToString(Template, *TemplatePath)) {
        UE_LOG(LogLinter, Error, TEXT("Could not load HTML report template."));
    }

    Template.ReplaceInline(TEXT("{% Report %}"), *ReportString);
    return Template;
}

ULinterNamingConvention* ULintRuleSet::GetNamingConvention() const {
    return NamingConvention.Get();
}

ULintResults* ULintRuleSet::LintPath(TArray<FString> AssetPaths, FScopedSlowTask* ParentScopedSlowTask /*= nullptr*/) const {
    // ReSharper disable once CppExpressionWithoutSideEffects
    NamingConvention.LoadSynchronous();

    ULintResults* Results = NewObject<ULintResults>();

    if (AssetPaths.Num() == 0) {
        AssetPaths.Push(TEXT("/Game"));
    }
    Results->Paths = AssetPaths;

    // Begin loading assets
    const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

    UE_LOG(LogLinter, Display, TEXT("Loading the asset registry..."));
    AssetRegistryModule.Get().SearchAllAssets(/*bSynchronousSearch =*/true);
    UE_LOG(LogLinter, Display, TEXT("Finished loading the asset registry. Loading assets..."));

    FARFilter ARFilter;
    ARFilter.bRecursivePaths = true;

    for (const FString& AssetPath : AssetPaths) {
        UE_LOG(LogLinter, Display, TEXT("Adding path \"%s\" to be linted."), *AssetPath);
        ARFilter.PackagePaths.Push(FName(*AssetPath));
    }

    AssetRegistryModule.Get().GetAssets(ARFilter, Results->CheckedAssets);

    TArray<FLintRunner*> LintRunners;
    TArray<FRunnableThread*> Threads;

    if (ParentScopedSlowTask != nullptr) {
        ParentScopedSlowTask->TotalAmountOfWork = Results->CheckedAssets.Num() + 2;
        ParentScopedSlowTask->CompletedWork = 0.0f;
    }

    for (const FAssetData& Asset : Results->CheckedAssets) {
        check(Asset.IsValid());
        UE_LOG(LogLinter, Verbose, TEXT("Creating Lint Thread for asset \"%s\"."), *Asset.AssetName.ToString());
        UObject* Object = Asset.GetAsset();
        check(Object != nullptr);

        FLintRunner* Runner = new FLintRunner(Object, this, &Results->Violations, ParentScopedSlowTask);
        check(Runner != nullptr);

        LintRunners.Add(Runner);

        if (Runner->RequiresGamethread()) {
            Runner->Run();
            // If we're given a scoped slow task, update its progress now...
            if (ParentScopedSlowTask != nullptr) {
                ParentScopedSlowTask->EnterProgressFrame(1.0f);
            }
        } else {
#if UE_VERSION_NEWER_THAN(5, 1, 0)
            Threads.Push(FRunnableThread::Create(Runner, *FString::Printf(TEXT("FLintRunner - %s"), *Asset.GetObjectPathString()), 0, TPri_Normal));
#else
            Threads.Push(FRunnableThread::Create(Runner, *FString::Printf(TEXT("FLintRunner - %s"), *Asset.ObjectPath.ToString()), 0, TPri_Normal));
#endif
            if (ParentScopedSlowTask != nullptr) {
                ParentScopedSlowTask->EnterProgressFrame(1.0f);
            }
        }
    }

    for (FRunnableThread* Thread : Threads) {
        Thread->WaitForCompletion();
    }

    if (ParentScopedSlowTask != nullptr) {
        ParentScopedSlowTask->EnterProgressFrame(1.0f, NSLOCTEXT("Linter", "ScanTaskFinished", "Tabulating Data..."));
    }

    // Count Errors and Warnings
    for (const FLintRuleViolation& Violation : Results->Violations) {
        if (Violation.ViolatedRule->GetDefaultObject<ULintRule>()->RuleSeverity <= ELintRuleSeverity::Error) {
            Results->Errors++;
        } else {
            Results->Warnings++;
        }
    }

    // Generate Result String
    Results->Result = FText::FormatNamed(
        FText::FromString("Linted {NumAssets} Assets: {NumWarnings} {NumWarnings}|plural(one=warning,other=warnings), {NumErrors} {NumErrors}|plural(one=error,other=errors)."),
        TEXT("NumAssets"), FText::FromString(FString::FromInt(Results->CheckedAssets.Num())),
        TEXT("NumWarnings"), FText::FromString(FString::FromInt(Results->Warnings)),
        TEXT("NumErrors"), FText::FromString(FString::FromInt(Results->Errors))
    );
    
    return Results;
}

const FLintRuleList* ULintRuleSet::GetLintRuleListForClass(const TSoftClassPtr<UObject> Class) const {
    UClass* SearchClass = Class.LoadSynchronous();
    while (SearchClass != nullptr) {
        const FLintRuleList* RuleList = ClassLintRulesMap.Find(SearchClass);
        if (RuleList != nullptr) {
            return RuleList;
        }

        // @HACK: If we reach UObject, find our hack rule for fallback
        if (SearchClass == UObject::StaticClass()) {
            const FLintRuleList* AnyObjectRuleList = ClassLintRulesMap.Find(UAnyObject_LinterDummyClass::StaticClass());
            return AnyObjectRuleList;
        }

        // Load our parent class in case we failed to get naming conventions
        SearchClass = SearchClass->GetSuperClass();
    }

    return nullptr;
}

bool FLintRuleList::RequiresGameThread() const {
    for (TSubclassOf<ULintRule> LintRuleSubClass : LintRules) {
        UClass* LintClass = LintRuleSubClass.Get();
        if (LintClass != nullptr) {
            const ULintRule* LintRule = GetDefault<ULintRule>(LintClass);
            if (LintRule != nullptr && LintRule->bRequiresGameThread) {
                return true;
            }
        }
    }

    return false;
}

bool FLintRuleList::PassesRules(UObject* ObjectToLint, const ULintRuleSet* ParentRuleSet, TArray<FLintRuleViolation>& OutRuleViolations) const {
    OutRuleViolations.Empty();

    bool bFailedAnyRule = false;
    for (TSubclassOf<ULintRule> LintRuleSubClass : LintRules) {
        UClass* LintClass = LintRuleSubClass.Get();
        if (LintClass != nullptr) {
            const ULintRule* LintRule = GetDefault<ULintRule>(LintClass);
            if (LintRule != nullptr) {
                TArray<FLintRuleViolation> ViolatedRules;
                bFailedAnyRule = !LintRule->PassesRule(ObjectToLint, ParentRuleSet, ViolatedRules) || bFailedAnyRule;
                OutRuleViolations.Append(ViolatedRules);
            }
        }
    }

    return !bFailedAnyRule;
}
