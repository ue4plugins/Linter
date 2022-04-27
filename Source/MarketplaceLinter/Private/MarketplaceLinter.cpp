// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "MarketplaceLinter.h"
#include "ISettingsModule.h"

#define LOCTEXT_NAMESPACE "FMarketplaceLinterModule"

void FMarketplaceLinterModule::StartupModule()
{

}

void FMarketplaceLinterModule::ShutdownModule()
{

}


#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FMarketplaceLinterModule, MarketplaceLinter)
DEFINE_LOG_CATEGORY(LogMarketplaceLinter);