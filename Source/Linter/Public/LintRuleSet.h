// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.
#pragma once

#include "LinterNamingConvention.h"
#include "Misc/ScopedSlowTask.h"
#include "LintRule.h"
#include "LintRuleSet.generated.h"


USTRUCT(BlueprintType)
struct LINTER_API FLintRuleList {
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = Default)
    TArray<TSubclassOf<ULintRule>> LintRules;

    bool RequiresGameThread() const;
    bool PassesRules(UObject* ObjectToLint, const ULintRuleSet* ParentRuleSet, TArray<FLintRuleViolation>& OutRuleViolations) const;
};


UCLASS(BlueprintType, Blueprintable)
class ULintResults : public UObject {
    GENERATED_BODY()

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lint")
    int32 Warnings = 0;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lint")
    int32 Errors = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lint")
    FText Result;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lint")
    TArray<FString> Paths;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lint")
    TArray<FAssetData> CheckedAssets;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lint")
    TArray<FLintRuleViolation> Violations;
    
    TArray<TSharedPtr<FLintRuleViolation>> GetSharedViolations() const;

    TSharedPtr<FJsonObject> GenerateJsonReport() const;

    UFUNCTION(BlueprintCallable, Category = "Lint")
    FString GenerateJsonReportString() const;
    
    UFUNCTION(BlueprintCallable, Category = "Lint")
    FString GenerateHTML() const;
};


UCLASS(BlueprintType, Blueprintable)
class LINTER_API ULintRuleSet : public UDataAsset {
    GENERATED_BODY()

public:
    // UFUNCTION(BlueprintCallable, Category = "Conventions")
    const FLintRuleList* GetLintRuleListForClass(TSoftClassPtr<UObject> Class) const;

    UFUNCTION(BlueprintCallable, Category = "Conventions")
    ULinterNamingConvention* GetNamingConvention() const;

    /** Invoke this with a list of asset paths to recursively lint all assets in paths. */
    // UFUNCTION(BlueprintCallable, Category = "Lint")
    ULintResults* LintPath(TArray<FString> AssetPaths, FScopedSlowTask* ParentScopedSlowTask = nullptr) const;

    UPROPERTY(EditDefaultsOnly, Category = "Marketplace")
    bool bShowMarketplacePublishingInfoInLintWizard = false;

    UPROPERTY(EditDefaultsOnly, Category = "Rules")
    FText RuleSetDescription;

    UPROPERTY(EditDefaultsOnly, Category = "Commandlet")
    FString NameForCommandlet;

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Rules")
    TSoftObjectPtr<ULinterNamingConvention> NamingConvention;

    UPROPERTY(EditDefaultsOnly, Category = "Rules")
    TMap<TSubclassOf<UObject>, FLintRuleList> ClassLintRulesMap;
};
