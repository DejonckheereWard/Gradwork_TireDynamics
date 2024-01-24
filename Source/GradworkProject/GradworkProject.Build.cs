// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GradworkProject : ModuleRules
{
	public GradworkProject(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "ChaosVehicles", "PhysicsCore", "Niagara" });
	}
}
