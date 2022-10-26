// Copyright Seven47 Software All Rights Reserved.

using UnrealBuildTool;

public class PolyZones_Plugin : ModuleRules
{
	public PolyZones_Plugin(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(new string[] { "Core" });
		PrivateDependencyModuleNames.AddRange(new string[] { "Projects", "CoreUObject", "Engine", "Slate", "SlateCore" });
		
		if (Target.bBuildEditor)
		{
			PublicDependencyModuleNames.AddRange(new string[] { "PolyZones_Editor" });
		}
	}
}
