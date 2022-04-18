// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.

#include "TemplateLinter.h"

#define LOCTEXT_NAMESPACE "FTemplateLinterModule"

void FTemplateLinterModule::StartupModule()
{
//	Super::StartupModule();
}

void FTemplateLinterModule::ShutdownModule()
{
//	Super::ShutdownModule();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FTemplateLinterModule, TemplateLinter)
DEFINE_LOG_CATEGORY(LogTemplateLinter);