#include "GAS/RDAbilityComponent.h"

#include "Character/AICharacter/RDBaseAICharacter.h"
#include "CollisionShape.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GAS/GA_Dash.h"
#include "GAS/GA_Grab.h"
#include "GameFramework/Character.h"

URDAbilityComponent::URDAbilityComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	DashAbilityClass = UGA_Dash::StaticClass();
	GrabAbilityClass = UGA_Grab::StaticClass();
}

void URDAbilityComponent::SetMainState(ERDAbilityMainState NewState)
{
	StateMachine.SetMainState(NewState);
}

bool URDAbilityComponent::CanTransitSkillState(ERDSkillFlowState FromState, ERDSkillFlowState ToState) const
{
	return StateMachine.CanTransitSkillState(FromState, ToState);
}

bool URDAbilityComponent::SetSkillState(ERDSkillFlowState NewState)
{
	return StateMachine.SetSkillState(NewState);
}

bool URDAbilityComponent::CanEnterDash() const
{
	return StateMachine.CanEnterDash();
}

bool URDAbilityComponent::CanEnterGrab() const
{
	return StateMachine.CanEnterGrab();
}

bool URDAbilityComponent::TryStartDash()
{
	if (!DashAbilityClass || !CanEnterDash())
	{
		return false;
	}

	if (!TryActivateAbilityByClass(DashAbilityClass))
	{
		return false;
	}

	return SetSkillState(ERDSkillFlowState::DashStartup);
}

bool URDAbilityComponent::CanStartGrab(AActor* Target) const
{
	const ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	const ARDBaseAICharacter* TargetCharacter = Cast<ARDBaseAICharacter>(Target);
	if (!OwnerCharacter || !TargetCharacter)
	{
		return false;
	}

	if (Target == OwnerCharacter || Target->IsPendingKillPending() || !TargetCharacter->CanBeGrabbed())
	{
		return false;
	}

	const FVector OwnerLocation = OwnerCharacter->GetActorLocation();
	const FVector TargetLocation = Target->GetActorLocation();
	const FVector ToTarget2D = FVector(TargetLocation.X - OwnerLocation.X, TargetLocation.Y - OwnerLocation.Y, 0.0f);
	if (ToTarget2D.IsNearlyZero())
	{
		return true;
	}

	if (ToTarget2D.SizeSquared() > FMath::Square(GrabRange))
	{
		return false;
	}

	if (FMath::Abs(TargetLocation.Z - OwnerLocation.Z) > MaxVerticalDelta)
	{
		return false;
	}

	const FVector Forward2D = OwnerCharacter->GetActorForwardVector().GetSafeNormal2D();
	const FVector DirToTarget = ToTarget2D.GetSafeNormal();
	const float Dot = FVector::DotProduct(Forward2D, DirToTarget);
	return Dot >= FMath::Cos(FMath::DegreesToRadians(GrabHalfAngleDeg));
}

AActor* URDAbilityComponent::FindBestGrabTarget() const
{
	const ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	UWorld* World = GetWorld();
	if (!OwnerCharacter || !World)
	{
		return nullptr;
	}

	TArray<FOverlapResult> Overlaps;
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerCharacter);

	const bool bHasOverlap = World->OverlapMultiByObjectType(
		Overlaps,
		OwnerCharacter->GetActorLocation(),
		FQuat::Identity,
		ObjectQueryParams,
		FCollisionShape::MakeSphere(GrabRange),
		QueryParams);

	if (!bHasOverlap)
	{
		return nullptr;
	}

	AActor* BestTarget = nullptr;
	float BestScore = -FLT_MAX;
	const FVector OwnerLocation = OwnerCharacter->GetActorLocation();
	const FVector Forward2D = OwnerCharacter->GetActorForwardVector().GetSafeNormal2D();

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Candidate = Overlap.GetActor();
		if (!CanStartGrab(Candidate))
		{
			continue;
		}

		const FVector CandidateLocation = Candidate->GetActorLocation();
		const FVector ToTarget2D = FVector(CandidateLocation.X - OwnerLocation.X, CandidateLocation.Y - OwnerLocation.Y, 0.0f);
		const float Distance2D = ToTarget2D.Size();
		const FVector DirToTarget = ToTarget2D.IsNearlyZero() ? Forward2D : ToTarget2D.GetSafeNormal();
		const float FrontDot = FVector::DotProduct(Forward2D, DirToTarget);
		const float Score = (FrontDot * 1000.0f) - Distance2D;

		if (Score > BestScore)
		{
			BestScore = Score;
			BestTarget = Candidate;
		}
	}

	return BestTarget;
}

void URDAbilityComponent::ClearCurrentGrabTarget()
{
	CurrentGrabTarget = nullptr;
}

bool URDAbilityComponent::TryStartGrab()
{
	if (!GrabAbilityClass || !CanEnterGrab())
	{
		CurrentGrabTarget = nullptr;
		return false;
	}

	AActor* Target = FindBestGrabTarget();
	if (!CanStartGrab(Target))
	{
		CurrentGrabTarget = nullptr;
		return false;
	}

	CurrentGrabTarget = Target;
	if (!TryActivateAbilityByClass(GrabAbilityClass))
	{
		CurrentGrabTarget = nullptr;
		return false;
	}

	return SetSkillState(ERDSkillFlowState::GrabAim);
}
