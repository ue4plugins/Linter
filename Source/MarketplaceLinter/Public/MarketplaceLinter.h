// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

DECLARE_LOG_CATEGORY_EXTERN(LogMarketplaceLinter, Verbose, All);

class MARKETPLACELINTER_API FMarketplaceLinterModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

};