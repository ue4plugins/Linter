// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "UI/SAssetLinkWidget.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "LinterStyle.h"
#include "SlateOptMacros.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SHyperlink.h"
#include "Misc/EngineVersionComparison.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SAssetLinkWidget::Construct(const FArguments& Args) {
    const float PaddingAmount = FLinterStyle::Get()->GetFloat("Linter.Padding");
    AssetData = Args._AssetData;

    ChildSlot
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot()
        .AutoWidth()
        [
            SNew(SHyperlink)
			.Text(FText::FromName(AssetData.Get().AssetName))
			.Padding(PaddingAmount)
			.OnNavigate_Lambda([&]() {
                                const FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
                                const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
                                TArray<FAssetData> AssetDatas;
#if UE_VERSION_NEWER_THAN(5, 1, 0)
                                AssetDatas.Push(AssetRegistryModule.Get().GetAssetByObjectPath(AssetData.Get().GetObjectPathString()));
#else
                                AssetDatas.Push(AssetRegistryModule.Get().GetAssetByObjectPath(AssetData.Get().ObjectPath));
#endif
                                ContentBrowserModule.Get().SyncBrowserToAssets(AssetDatas);
                            })
        ]
    ];
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
