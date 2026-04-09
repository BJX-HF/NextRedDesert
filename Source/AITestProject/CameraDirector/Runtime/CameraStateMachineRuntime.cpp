#include "CameraDirector/Runtime/CameraStateMachineRuntime.h"

#include "CameraDirector/Assets/CameraStateMachineAsset.h"

void FCameraStateMachineRuntime::Initialize(UCameraStateMachineAsset* InAsset)
{
	Asset = InAsset;
	CurrentStateName = Asset ? Asset->EntryState : NAME_None;
	PreviousStateName = NAME_None;
	BlendElapsed = 0.0f;
	bIsBlending = false;
	CurrentResult = FCameraResult();
}

void FCameraStateMachineRuntime::ForceSetState(FName StateName)
{
	if (!Asset || !Asset->FindStateByName(StateName))
	{
		return;
	}

	PreviousStateName = CurrentStateName;
	CurrentStateName = StateName;
	BlendElapsed = 0.0f;
	bIsBlending = false;
}

void FCameraStateMachineRuntime::Tick(const FCameraStateContext& Context)
{
	if (!Asset)
	{
		return;
	}

	if (CurrentStateName.IsNone())
	{
		CurrentStateName = Asset->EntryState;
	}

	if (const FCameraTransitionDefinition* Transition = FindBestTransition(Context))
	{
		StartTransition(*Transition);
	}

	const FCameraStateDefinition* CurrentState = Asset->FindStateByName(CurrentStateName);
	if (!CurrentState)
	{
		return;
	}

	FCameraResult DesiredResult = BuildCameraResultFromState(*CurrentState, Context);

	if (bIsBlending && !PreviousStateName.IsNone())
	{
		if (const FCameraStateDefinition* PreviousState = Asset->FindStateByName(PreviousStateName))
		{
			const FCameraResult PreviousResult = BuildCameraResultFromState(*PreviousState, Context);
			BlendElapsed += Context.DeltaTime;

			const float LinearAlpha = ActiveBlendSettings.BlendTime <= KINDA_SMALL_NUMBER
				? 1.0f
				: FMath::Clamp(BlendElapsed / ActiveBlendSettings.BlendTime, 0.0f, 1.0f);
			const float BlendAlpha = ActiveBlendSettings.EvaluateAlpha(LinearAlpha);
			DesiredResult = FCameraResult::Blend(PreviousResult, DesiredResult, BlendAlpha);

			if (LinearAlpha >= 1.0f)
			{
				bIsBlending = false;
				PreviousStateName = NAME_None;
			}
		}
	}

	CurrentResult = DesiredResult;
}

void FCameraStateMachineRuntime::StartTransition(const FCameraTransitionDefinition& Transition)
{
	if (Transition.ToState == CurrentStateName)
	{
		return;
	}

	PreviousStateName = CurrentStateName;
	CurrentStateName = Transition.ToState;
	ActiveBlendSettings = Transition.BlendSettings;
	BlendElapsed = 0.0f;
	bIsBlending = true;
}

FCameraResult FCameraStateMachineRuntime::BuildCameraResultFromState(const FCameraStateDefinition& State, const FCameraStateContext& Context) const
{
	const FCameraBasicParams& Params = State.Params;

	FCameraResult Result;
	Result.TargetArmLength = Params.TargetArmLength;
	Result.SocketOffset = Params.SocketOffset;
	Result.TargetOffset = Params.TargetOffset;
	Result.FOV = Params.FOV;
	Result.CameraLagSpeed = Params.CameraLagSpeed;
	Result.CameraRotationLagSpeed = Params.CameraRotationLagSpeed;
	Result.bUsePawnControlRotation = Params.bUsePawnControlRotation;
	Result.CameraRotation = Context.ControlRotation + FRotator(Params.PitchOffset, Params.YawOffset, 0.0f);

	const FVector PivotLocation = Context.ActorLocation + Params.TargetOffset;
	const FVector RotatedSocketOffset = FRotationMatrix(Result.CameraRotation).TransformVector(Params.SocketOffset);
	Result.CameraLocation = PivotLocation + RotatedSocketOffset - (Result.CameraRotation.Vector() * Params.TargetArmLength);
	return Result;
}

const FCameraTransitionDefinition* FCameraStateMachineRuntime::FindBestTransition(const FCameraStateContext& Context) const
{
	if (!Asset || CurrentStateName.IsNone())
	{
		return nullptr;
	}

	TArray<const FCameraTransitionDefinition*> Candidates = Asset->GetOutgoingTransitions(CurrentStateName);
	const FCameraTransitionDefinition* BestTransition = nullptr;

	for (const FCameraTransitionDefinition* Transition : Candidates)
	{
		if (!Transition || (bIsBlending && !Transition->bCanInterrupt) || !Transition->Rule.Evaluate(Context))
		{
			continue;
		}

		if (!BestTransition || Transition->Priority > BestTransition->Priority)
		{
			BestTransition = Transition;
		}
	}

	return BestTransition;
}
