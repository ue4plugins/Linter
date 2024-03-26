// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.
#include "LintRules/LintRule_Texture_Size_NotTooBig.h"

#include "Linter.h"
#include "LintRuleSet.h"

ULintRule_Texture_Size_NotTooBig::ULintRule_Texture_Size_NotTooBig(const FObjectInitializer& ObjectInitializer) :
    Super(ObjectInitializer) {}

bool ULintRule_Texture_Size_NotTooBig::PassesRule(UObject* ObjectToLint, const ULintRuleSet* ParentRuleSet, TArray<FLintRuleViolation>& OutRuleViolations) const {
    // If we aren't a texture, abort
    if (Cast<UTexture2D>(ObjectToLint) == nullptr) {
        // @TODO: Bubble up some sort of configuration error?
        return true;
    }

    return Super::PassesRule(ObjectToLint, ParentRuleSet, OutRuleViolations);
}

bool ULintRule_Texture_Size_NotTooBig::PassesRule_Internal_Implementation(UObject* ObjectToLint, const ULintRuleSet* ParentRuleSet, TArray<FLintRuleViolation>& OutRuleViolations) const {
    const UTexture2D* Texture = CastChecked<UTexture2D>(ObjectToLint);

    // ToDo: Make Texture->GetSizeX() work again (possibly Bug in 5.3?)
    const FTexturePlatformData* PlatformData = Texture->GetPlatformData();
    if (!PlatformData) {
        UE_LOG(LogLinter, Warning, TEXT("Could not get Platform Data for Texture!"))
        return true;
    }
    
    int32 TexSizeX = PlatformData->SizeX;
    int32 TexSizeY = PlatformData->SizeY;

    // Check to see if textures are too big
    if (TexSizeX > MaxTextureSizeX || TexSizeY > MaxTextureSizeY) {
        const FText RecommendedAction = NSLOCTEXT("Linter", "LintRule_Texture_Size_NotTooBig_TooBig", "Please shrink your textures dimensions so that they fit within {0}x{1} pixels.");
        OutRuleViolations.Push(FLintRuleViolation(ObjectToLint, GetClass(), FText::FormatOrdered(RecommendedAction, MaxTextureSizeX, MaxTextureSizeY)));
        return false;
    }

    return true;
}
