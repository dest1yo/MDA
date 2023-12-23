// Copyright 2023 dest1yo. All Rights Reserved.

using UnrealBuildTool;

public class MDAEditor : ModuleRules
{
	public MDAEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
            }
			);


		PrivateIncludePaths.AddRange(
			new string[] {
                "MDARuntime/Private",
                "MDARuntime/Public",
                // ... add other private include paths required here ...
            }
            );


		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				// ... add other public dependencies that you statically link with here ...
			}
			);


        PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				// ... add private dependencies that you statically link with here ...	
                "MDARuntime",

                "AnimGraph",
				"AnimationBlueprintEditor", // SGraphNodeMDA.h needed
				"ToolMenus",
				"Kismet",
				"UnrealEd",
				"BlueprintGraph",
				"GraphEditor",

			}
			);


		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}