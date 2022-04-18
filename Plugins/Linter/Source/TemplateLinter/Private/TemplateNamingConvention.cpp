// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.
#include "TemplateNamingConvention.h"

#include "IAssetTypeActions.h"
#include "AssetToolsModule.h"
#include "Misc/BlacklistNames.h"

UTemplateNamingConvention::UTemplateNamingConvention(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	if(HasAnyFlags(EObjectFlags::RF_ClassDefaultObject))
	{
		return;
	}

	auto AddNamingConvention = [&](const FString& ObjectPath, const FString& Prefix = TEXT(""), const FString& Suffix = TEXT(""), const FName& Variant = NAME_None) {
		ClassNamingConventions.Push(FLinterNamingConventionInfo(TSoftClassPtr<UObject>(FSoftObjectPath(ObjectPath)), Prefix, Suffix, Variant));
	};

	// Add all type action asset, reference SFilterList::MakeAddFilterMenu
	{
		FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));

		// Get the browser type maps
		TArray<TWeakPtr<IAssetTypeActions>> AssetTypeActionsList;
		AssetToolsModule.Get().GetAssetTypeActionsList(AssetTypeActionsList);

		// Sort the list
		struct FCompareIAssetTypeActions
		{
			FORCEINLINE bool operator()(const TWeakPtr<IAssetTypeActions>& A, const TWeakPtr<IAssetTypeActions>& B) const
			{
				return A.Pin()->GetName().CompareTo(B.Pin()->GetName()) == -1;
			}
		};
		AssetTypeActionsList.Sort(FCompareIAssetTypeActions());

		TSharedRef<FBlacklistNames> AssetClassBlacklist = AssetToolsModule.Get().GetAssetClassBlacklist();

		// For every asset type, add naming convention
		for (int32 ClassIdx = 0; ClassIdx < AssetTypeActionsList.Num(); ++ClassIdx)
		{
			const TWeakPtr<IAssetTypeActions>& WeakTypeActions = AssetTypeActionsList[ClassIdx];
			if (WeakTypeActions.IsValid())
			{
				TSharedPtr<IAssetTypeActions> TypeActions = WeakTypeActions.Pin();
				if (ensure(TypeActions.IsValid()) && TypeActions->CanFilter())
				{
					UClass* SupportedClass = TypeActions->GetSupportedClass();
					if (SupportedClass && AssetClassBlacklist->PassesFilter(SupportedClass->GetFName()))
					{
						AddNamingConvention(SupportedClass->GetPathName());
					}
				}
			}
		}
	}

	// Add missing asset and variant
	{
		// Animation
		AddNamingConvention(TEXT("/Script/Engine.AnimLayerInterface"));
		AddNamingConvention(TEXT("/Script/Engine.MorphTarget"));

		// Artificial Intelligence
		AddNamingConvention(TEXT("/Script/Engine.AIController"));
		AddNamingConvention(TEXT("/Script/Engine.BTDecorator"));
		AddNamingConvention(TEXT("/Script/Engine.BTService"));
		AddNamingConvention(TEXT("/Script/Engine.BTTaskNode"));

		// Blueprints
		AddNamingConvention(TEXT("/Script/Engine.Blueprint"), "", "", "MacroLibrary");
		AddNamingConvention(TEXT("/Script/Engine.Blueprint"), "", "", "Interface");
		AddNamingConvention(TEXT("/Script/Engine.BlueprintFunctionLibrary"));

		// Materials
		AddNamingConvention(TEXT("/Script/Engine.Material"), "", "", "PostProcess");
		AddNamingConvention(TEXT("/Script/Engine.MaterialInstanceConstant"), "", "", "PostProcess");
	}

	SortConventions();
}

