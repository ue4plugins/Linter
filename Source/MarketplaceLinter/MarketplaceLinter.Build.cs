// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MarketplaceLinter : ModuleRules {
	public MarketplaceLinter(ReadOnlyTargetRules Target) : base(Target) {
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new[] { "Core", "Linter" });
		PrivateDependencyModuleNames.AddRange(new[] {
			"CoreUObject", "Engine", "Slate", "SlateCore", "RenderCore", "UnrealEd", "GraphEditor", "AssetTools", 
			"EditorStyle", "Projects", "BlueprintGraph", "InputCore", "StandaloneRenderer", "PropertyEditor", 
			"LevelEditor", "LauncherPlatform", "AppFramework", "DesktopPlatform", "UATHelper"
		});
	}
}