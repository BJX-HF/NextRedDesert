#include "GAS/GA_Grab.h"

#include "Character/AICharacter/RDBaseAICharacter.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/RDAbilityComponent.h"
#include "TimerManager.h"

UGA_Grab::UGA_Grab()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UGA_Grab::ActivateAbility(
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
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	ARDBaseAICharacter* TargetCharacter = RDAbilityComponent ? Cast<ARDBaseAICharacter>(RDAbilityComponent->GetCurrentGrabTarget()) : nullptr;
	if (!RDAbilityComponent || !OwnerCharacter || !TargetCharacter || !TargetCharacter->CanBeGrabbed())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!RDAbilityComponent->SetSkillState(ERDSkillFlowState::GrabLatch))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	CachedOwnerCharacter = OwnerCharacter;
	GrabbedTarget = TargetCharacter;
	bHasExecutedThrow = false;

	TargetCharacter->SetCanBeGrabbed(false);

	if (USkeletalMeshComponent* OwnerMesh = OwnerCharacter->GetMesh())
	{
		TargetCharacter->AttachToComponent(OwnerMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, GrabAttachSocketName);
		TargetCharacter->AddActorLocalOffset(GrabAttachOffset);
	}
	else
	{
		TargetCharacter->AttachToActor(OwnerCharacter, FAttachmentTransformRules::KeepWorldTransform);
	}

	if (bDisableTargetMovementWhileHeld)
	{
		if (UCharacterMovementComponent* TargetMovement = TargetCharacter->GetCharacterMovement())
		{
			TargetMovement->DisableMovement();
		}
	}

	if (!RDAbilityComponent->SetSkillState(ERDSkillFlowState::GrabHold))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(ThrowTimerHandle, this, &UGA_Grab::ExecuteThrow, HoldDuration, false);
	}
	else
	{
		ExecuteThrow();
	}
}

void UGA_Grab::ExecuteThrow()
{
	if (!IsActive())
	{
		return;
	}

	if (URDAbilityComponent* RDAbilityComponent = Cast<URDAbilityComponent>(GetAbilitySystemComponentFromActorInfo()))
	{
		RDAbilityComponent->SetSkillState(ERDSkillFlowState::GrabThrow);
	}

	ReleaseTarget(true);
	bHasExecutedThrow = true;
	K2_EndAbility();
}

void UGA_Grab::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	ClearThrowTimer();

	if (!bHasExecutedThrow)
	{
		ReleaseTarget(false);
	}

	if (URDAbilityComponent* RDAbilityComponent = Cast<URDAbilityComponent>(ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr))
	{
		RDAbilityComponent->SetSkillState(ERDSkillFlowState::SkillRecovery);
		RDAbilityComponent->SetSkillState(ERDSkillFlowState::None);
		RDAbilityComponent->ClearCurrentGrabTarget();
	}

	CachedOwnerCharacter = nullptr;
	GrabbedTarget = nullptr;
	bHasExecutedThrow = false;

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Grab::ClearThrowTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ThrowTimerHandle);
	}
}

void UGA_Grab::ReleaseTarget(bool bApplyThrow)
{
	ARDBaseAICharacter* TargetCharacter = GrabbedTarget.Get();
	ACharacter* OwnerCharacter = CachedOwnerCharacter.Get();
	if (!TargetCharacter)
	{
		return;
	}

	TargetCharacter->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	if (UCharacterMovementComponent* TargetMovement = TargetCharacter->GetCharacterMovement())
	{
		TargetMovement->SetMovementMode(MOVE_Falling);
	}

	if (bApplyThrow && OwnerCharacter)
	{
		const FVector ThrowVelocity = OwnerCharacter->GetActorForwardVector().GetSafeNormal2D() * ThrowForwardStrength + FVector::UpVector * ThrowUpStrength;
		TargetCharacter->LaunchCharacter(ThrowVelocity, true, true);
	}

	TargetCharacter->SetCanBeGrabbed(true);
}
