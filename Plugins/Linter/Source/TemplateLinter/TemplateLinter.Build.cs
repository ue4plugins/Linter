// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.

using UnrealBuildTool;

public class TemplateLinter : ModuleRules
{
    public TemplateLinter(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
                "Engine",
				"Linter"
			}
		);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
            {

			}
		);
    }
}
