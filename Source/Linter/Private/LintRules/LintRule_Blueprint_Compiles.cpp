// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.
#include "LintRules/LintRule_Blueprint_Compiles.h"
#include "LintRuleSet.h"
#include "Sound/SoundWave.h"
#include "Engine/Blueprint.h"

ULintRule_Blueprint_Compiles::ULintRule_Blueprint_Compiles(const FObjectInitializer& ObjectInitializer) :
    Super(ObjectInitializer) {}

bool ULintRule_Blueprint_Compiles::PassesRule_Internal_Implementation(UObject* ObjectToLint, const ULintRuleSet* ParentRuleSet, TArray<FLintRuleViolation>& OutRuleViolations) const {
    const UBlueprint* Blueprint = CastChecked<UBlueprint>(ObjectToLint);

    switch (Blueprint->Status) {
        case BS_BeingCreated:
        case BS_Dirty:
        case BS_Unknown:
        case BS_UpToDate: return true;
        case BS_Error:
        case BS_UpToDateWithWarnings: OutRuleViolations.Push(FLintRuleViolation(ObjectToLint, GetClass()));
            return false;
        default: return true;
    }
}
