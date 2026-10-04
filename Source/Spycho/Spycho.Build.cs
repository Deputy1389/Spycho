using UnrealBuildTool;
public class Spycho : ModuleRules
{
    public Spycho(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new string[] {"Core", "CoreUObject", "Engine", "InputCore", "PhysicsCore", "AIModule", "UMG", "Slate", "SlateCore"});
    }
}
