// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.
#include "LintRules/LintRule_Texture_Size_PowerOfTwo.h"

#include "Linter.h"
#include "LintRuleSet.h"

ULintRule_Texture_Size_PowerOfTwo::ULintRule_Texture_Size_PowerOfTwo(const FObjectInitializer& ObjectInitializer) :
    Super(ObjectInitializer) {
    IgnoreTexturesInTheseGroups.Add(TEXTUREGROUP_UI);
}

bool ULintRule_Texture_Size_PowerOfTwo::PassesRule(UObject* ObjectToLint, const ULintRuleSet* ParentRuleSet, TArray<FLintRuleViolation>& OutRuleViolations) const {
    // If we aren't a texture, abort
    if (Cast<UTexture2D>(ObjectToLint) == nullptr) {
        // @TODO: Bubble up some sort of configuration error?
        return true;
    }

    // If we're to ignore this texture LOD group, abort
    if (IgnoreTexturesInTheseGroups.Contains(Cast<UTexture2D>(ObjectToLint)->LODGroup)) {
        return true;
    }

    return Super::PassesRule(ObjectToLint, ParentRuleSet, OutRuleViolations);
}

bool ULintRule_Texture_Size_PowerOfTwo::PassesRule_Internal_Implementation(UObject* ObjectToLint, const ULintRuleSet* ParentRuleSet, TArray<FLintRuleViolation>& OutRuleViolations) const {
    const UTexture2D* Texture = CastChecked<UTexture2D>(ObjectToLint);

    // ToDo: Make Texture->GetSizeX() work again (possibly Bug in 5.3?)
    const FTexturePlatformData* PlatformData = Texture->GetPlatformData();
    if (!PlatformData) {
        UE_LOG(LogLinter, Warning, TEXT("Could not get Platform Data for Texture!"))
        return true;
    }
    
    int32 TexSizeX = PlatformData->SizeX;
    int32 TexSizeY = PlatformData->SizeY;

    const bool bXFail = ((TexSizeX & (TexSizeX - 1)) != 0);
    const bool bYFail = ((TexSizeY & (TexSizeY - 1)) != 0);

    const UEnum* TextureGroupEnum = StaticEnum<TextureGroup>();
    FString IgnoredLODGroupNames;

    for (TEnumAsByte<TextureGroup> LODGroup : IgnoreTexturesInTheseGroups) {
        IgnoredLODGroupNames += TextureGroupEnum->GetMetaData(TEXT("DisplayName"), LODGroup) + TEXT(", ");
    }
    IgnoredLODGroupNames.RemoveFromEnd(TEXT(", "));

    FText IgnoredLODGroupTip = IgnoredLODGroupNames.Len() > 0 ? FText::FormatOrdered(NSLOCTEXT("Linter", "LintRule_Texture_Size_PowerOfTwo_AllowedLODGroups", ". Alternatively, assign this texture to one of these LOD Groups: [{0}]"), FText::FromString(IgnoredLODGroupNames)) : FText::GetEmpty();

    if (bXFail || bYFail) {
        FText RecommendedAction;
        if (bXFail && bYFail) {
            RecommendedAction = FText::FormatOrdered(NSLOCTEXT("Linter", "LintRule_Texture_Size_PowerOfTwo_Fail_XY", "Please fix the width and height of this texture, currently {0} by {1}{2}"), TexSizeX, TexSizeY, IgnoredLODGroupTip);
        } else if (bXFail) {
            RecommendedAction = FText::FormatOrdered(NSLOCTEXT("Linter", "LintRule_Texture_Size_PowerOfTwo_Fail_X", "Please fix the width of this texture, currently {0}{1}"), TexSizeX, IgnoredLODGroupTip);
        } else if (bYFail) {
            RecommendedAction = FText::FormatOrdered(NSLOCTEXT("Linter", "LintRule_Texture_Size_PowerOfTwo_Fail_Y", "Please fix the height of this texture, currently {0}{1}"), TexSizeY, IgnoredLODGroupTip);
        }

        OutRuleViolations.Push(FLintRuleViolation(ObjectToLint, GetClass(), RecommendedAction));
        return false;
    }

    return true;
}
