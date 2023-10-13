#include "LintRuleSet.h"

#include "AnyObject_LinterDummyClass.h"
#include "LintRunner.h"
#include "Linter.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Modules/ModuleManager.h"
#include "HAL/RunnableThread.h"

ULintRuleSet::ULintRuleSet(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

ULinterNamingConvention* ULintRuleSet::GetNamingConvention() const
{
	return NamingConvention.Get();
}

TArray<FLintRuleViolation> ULintRuleSet::LintPath(TArray<FString> AssetPaths, FScopedSlowTask* ParentScopedSlowTask /*= nullptr*/) const
{
	// ReSharper disable once CppExpressionWithoutSideEffects
	NamingConvention.LoadSynchronous();

	TArray<FLintRuleViolation> RuleViolations;

	if (AssetPaths.Num() == 0)
	{
		AssetPaths.Push(TEXT("/Game"));
	}

	// Begin loading assets
	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	UE_LOG(LogLinter, Display, TEXT("Loading the asset registry..."));
	AssetRegistryModule.Get().SearchAllAssets(/*bSynchronousSearch =*/true);
	UE_LOG(LogLinter, Display, TEXT("Finished loading the asset registry. Loading assets..."));

	TArray<FAssetData> AssetList;

	FARFilter ARFilter;
	ARFilter.bRecursivePaths = true;

	for (const FString& AssetPath : AssetPaths)
	{
		UE_LOG(LogLinter, Display, TEXT("Adding path \"%s\" to be linted."), *AssetPath);
		ARFilter.PackagePaths.Push(FName(*AssetPath));
	}

	AssetRegistryModule.Get().GetAssets(ARFilter, AssetList);

	TArray<FLintRunner*> LintRunners;
	TArray<FRunnableThread*> Threads;

	if (ParentScopedSlowTask != nullptr)
	{
		ParentScopedSlowTask->TotalAmountOfWork = AssetList.Num() + 2;
		ParentScopedSlowTask->CompletedWork = 0.0f;
	}

	for (FAssetData const& Asset : AssetList)
	{
		check(Asset.IsValid());
		UE_LOG(LogLinter, Verbose, TEXT("Creating Lint Thread for asset \"%s\"."), *Asset.AssetName.ToString());
		UObject* Object = Asset.GetAsset();
		check(Object != nullptr);

		FLintRunner* Runner = new FLintRunner(Object, this, &RuleViolations, ParentScopedSlowTask);
		check(Runner != nullptr);

		LintRunners.Add(Runner);

		if (Runner->RequiresGamethread())
		{
			Runner->Run();
			// If we're given a scoped slow task, update its progress now...
			if (ParentScopedSlowTask != nullptr)
			{
				ParentScopedSlowTask->EnterProgressFrame(1.0f);
			}
		}
		else
		{
#if UE_VERSION_NEWER_THAN(5, 1, 0)
			Threads.Push(FRunnableThread::Create(Runner, *FString::Printf(TEXT("FLintRunner - %s"), *Asset.GetObjectPathString()), 0, TPri_Normal));
#else
			Threads.Push(FRunnableThread::Create(Runner, *FString::Printf(TEXT("FLintRunner - %s"), *Asset.ObjectPath.ToString()), 0, TPri_Normal));
#endif
			if (ParentScopedSlowTask != nullptr)
			{
				ParentScopedSlowTask->EnterProgressFrame(1.0f);
			}
		}
	}

	for (FRunnableThread* Thread : Threads)
	{
		Thread->WaitForCompletion();
	}

	if (ParentScopedSlowTask != nullptr)
	{
		ParentScopedSlowTask->EnterProgressFrame(1.0f, NSLOCTEXT("Linter", "ScanTaskFinished", "Tabulating Data..."));
	}

	return RuleViolations;
}

TArray<TSharedPtr<FLintRuleViolation>> ULintRuleSet::LintPathShared(const TArray<FString> AssetPaths, FScopedSlowTask* ParentScopedSlowTask /*= nullptr*/) const
{
	TArray<FLintRuleViolation> RuleViolations = LintPath(AssetPaths, ParentScopedSlowTask);

	TArray<TSharedPtr<FLintRuleViolation>> SharedRuleViolations;
	for (const FLintRuleViolation Violation : RuleViolations)
	{
		TSharedPtr<FLintRuleViolation> SharedViolation = MakeShared<FLintRuleViolation>(Violation);
		SharedViolation->PopulateAssetData();
		SharedRuleViolations.Push(SharedViolation);
	}

	return SharedRuleViolations;
}

const FLintRuleList* ULintRuleSet::GetLintRuleListForClass(const TSoftClassPtr<UObject> Class) const
{
	UClass* SearchClass = Class.LoadSynchronous();
	while (SearchClass != nullptr)
	{
		const FLintRuleList* RuleList = ClassLintRulesMap.Find(SearchClass);
		if (RuleList != nullptr)
		{
			return RuleList;
		}

		// @HACK: If we reach UObject, find our hack rule for fallback
		if (SearchClass == UObject::StaticClass())
		{
			const FLintRuleList* AnyObjectRuleList = ClassLintRulesMap.Find(UAnyObject_LinterDummyClass::StaticClass());
			return AnyObjectRuleList;
		}

		// Load our parent class in case we failed to get naming conventions
		SearchClass = SearchClass->GetSuperClass();
	}

	return nullptr;
}

bool FLintRuleList::RequiresGameThread() const
{
	for (TSubclassOf<ULintRule> LintRuleSubClass : LintRules)
	{
		UClass* LintClass = LintRuleSubClass.Get();
		if (LintClass != nullptr)
		{
			const ULintRule* LintRule = GetDefault<ULintRule>(LintClass);
			if (LintRule != nullptr && LintRule->bRequiresGameThread)
			{
				return true;
			}
		}
	}

	return false;
}

bool FLintRuleList::PassesRules(UObject* ObjectToLint, const ULintRuleSet* ParentRuleSet, TArray<FLintRuleViolation>& OutRuleViolations) const
{
	OutRuleViolations.Empty();

	bool bFailedAnyRule = false;
	for (TSubclassOf<ULintRule> LintRuleSubClass : LintRules)
	{
		UClass* LintClass = LintRuleSubClass.Get();
		if (LintClass != nullptr)
		{
			const ULintRule* LintRule = GetDefault<ULintRule>(LintClass);
			if (LintRule != nullptr)
			{
				TArray<FLintRuleViolation> ViolatedRules;
				bFailedAnyRule = !LintRule->PassesRule(ObjectToLint, ParentRuleSet, ViolatedRules) || bFailedAnyRule;
				OutRuleViolations.Append(ViolatedRules);
			}
		}
	}

	return !bFailedAnyRule;
}