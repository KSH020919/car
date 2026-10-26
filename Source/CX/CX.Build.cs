using UnrealBuildTool;

/// 기존 차량과 입력 및 과제 화면에 필요한 모듈을 지정합니다.
public class CX : ModuleRules
{
    /// 프로젝트의 빌드 의존성과 헤더 검색 경로를 구성합니다.
    public CX(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicIncludePaths.Add(ModuleDirectory);
        PublicDependencyModuleNames.AddRange(new string[] {
            "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput",
            "ChaosVehicles", "PhysicsCore", "UMG", "Slate", "SlateCore" });
    }
}
