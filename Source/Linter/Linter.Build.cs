// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.

using UnrealBuildTool;

public class Linter : ModuleRules {
	public Linter(ReadOnlyTargetRules Target) : base(Target) {
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new[] { "Core" });
		PrivateDependencyModuleNames.AddRange(new[] {
			"CoreUObject", "Engine", "Slate", "SlateCore", "AppFramework",
			"InputCore", "UnrealEd", "GraphEditor", "AssetTools", "EditorStyle", "BlueprintGraph", "PropertyEditor",
			"LauncherPlatform", "Projects", "DesktopPlatform", "Json", "UATHelper", "ToolMenus", "ContentBrowser", "ContentBrowserData"
		});

		PublicIncludePathModuleNames.Add("Launch");

#if UE_4_20_OR_LATER
		PublicDefinitions.Add("UE_4_20_OR_LATER=1");
#endif
	}
}