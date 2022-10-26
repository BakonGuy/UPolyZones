using UnrealBuildTool;

public class PolyZones_Editor : ModuleRules
{
    public PolyZones_Editor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "GeometryScriptingEditor", "GeometryScriptingCore" });
        PrivateDependencyModuleNames.AddRange(new string[] { "Projects", "CoreUObject", "Engine", "Slate", "SlateCore" });
    }
}