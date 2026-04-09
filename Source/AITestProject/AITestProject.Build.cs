// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class AITestProject : ModuleRules
{
	public AITestProject(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"AITestProject",
			"AITestProject/Variant_Platforming",
			"AITestProject/Variant_Platforming/Animation",
			"AITestProject/Variant_Combat",
			"AITestProject/Variant_Combat/AI",
			"AITestProject/Variant_Combat/Animation",
			"AITestProject/Variant_Combat/Gameplay",
			"AITestProject/Variant_Combat/Interfaces",
			"AITestProject/Variant_Combat/UI",
			"AITestProject/Variant_SideScrolling",
			"AITestProject/Variant_SideScrolling/AI",
			"AITestProject/Variant_SideScrolling/Gameplay",
			"AITestProject/Variant_SideScrolling/Interfaces",
			"AITestProject/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
