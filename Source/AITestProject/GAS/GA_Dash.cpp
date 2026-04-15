#include "GAS/GA_Dash.h"

#include "GAS/RDAbilityComponent.h"
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

	URDAbilityComponent* RDAbilityComponent = Cast<URDAbilityComponent>(ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr);
	if (RDAbilityComponent)
	{
		RDAbilityComponent->SetSkillState(ERDSkillFlowState::DashActive);
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

void UGA_Dash::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	if (URDAbilityComponent* RDAbilityComponent = Cast<URDAbilityComponent>(ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr))
	{
		if (!RDAbilityComponent->SetSkillState(ERDSkillFlowState::DashRecovery))
		{
			RDAbilityComponent->SetSkillState(ERDSkillFlowState::SkillRecovery);
		}
		RDAbilityComponent->SetSkillState(ERDSkillFlowState::None);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

