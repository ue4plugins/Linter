// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "LinterStyle.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"

class SAssetLinkWidget : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SAssetLinkWidget)
	{
	}

	SLATE_ATTRIBUTE(FAssetData, AssetData)

	SLATE_END_ARGS()

	TAttribute<FAssetData> AssetData;

public:

	void Construct(const FArguments& Args);
};