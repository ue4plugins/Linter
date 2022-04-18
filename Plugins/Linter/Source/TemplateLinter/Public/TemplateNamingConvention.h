// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "LinterNamingConvention.h"
#include "TemplateNamingConvention.generated.h"

UCLASS()
class UTemplateNamingConvention : public ULinterNamingConvention
{
	GENERATED_BODY()

public:

	UTemplateNamingConvention(const FObjectInitializer& ObjectInitializer);

};
