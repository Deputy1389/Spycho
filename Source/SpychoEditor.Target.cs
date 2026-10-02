using UnrealBuildTool;
public class SpychoEditorTarget : TargetRules
{
    public SpychoEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V7;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        ExtraModuleNames.Add("Spycho");
    }
}
