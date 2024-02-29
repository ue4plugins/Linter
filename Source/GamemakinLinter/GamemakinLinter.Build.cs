// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.

using UnrealBuildTool;

public class GamemakinLinter : ModuleRules {
	public GamemakinLinter(ReadOnlyTargetRules Target) : base(Target) {
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "Linter" });
	}
}