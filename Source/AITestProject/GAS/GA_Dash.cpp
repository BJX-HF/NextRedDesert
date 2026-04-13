#include "GAS/GA_Dash.h"

#include "GameFramework/Character.h"

UGA_Dash::UGA_Dash()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UGA_Dash::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ACharacter* AvatarCharacter = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (AvatarCharacter)
	{
		FVector DashVelocity = AvatarCharacter->GetActorForwardVector().GetSafeNormal2D() * DashStrength;
		if (!bOverrideZ)
		{
			DashVelocity.Z = AvatarCharacter->GetVelocity().Z;
		}
		AvatarCharacter->LaunchCharacter(DashVelocity, true, bOverrideZ);
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
