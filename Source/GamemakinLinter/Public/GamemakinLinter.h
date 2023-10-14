// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.
#pragma once

DECLARE_LOG_CATEGORY_EXTERN(LogGamemakinLinter, Verbose, All);

class GAMEMAKINLINTER_API FGamemakinLinterModule : public IModuleInterface {
public:
    virtual bool SupportsDynamicReloading() override {
        return false;
    }
};
