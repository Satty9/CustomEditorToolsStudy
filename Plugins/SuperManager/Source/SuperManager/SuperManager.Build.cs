// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

// shorter version
using System.IO;

public class SuperManager : ModuleRules
{
	public SuperManager(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// long version
				//System.IO.Path.GetFullPath(Target.RelativeEnginePath) + ("Source/Editor/Blutility/Private"),
				
				// shorter version
				Path.Combine(EngineDirectory, "Source/Editor/Blutility/Private"),
				//Path.Combine(EngineDirectory, "Plugins/Edtor/EditorScriptingUtilities/Source/EditorScriptingUtilities/Private")
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				// ... add other public dependencies that you statically link with here ...
				"Blutility",
				"EditorScriptingUtilities",
				"Niagara",
				"UMG",
				"UnrealEd",
				"InputCore"
				/*"AssetToolsModule"*/
				/*"ContentBrowser",*/
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore", "EditorScriptingUtilities",
				// ... add private dependencies that you statically link with here ...	
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
