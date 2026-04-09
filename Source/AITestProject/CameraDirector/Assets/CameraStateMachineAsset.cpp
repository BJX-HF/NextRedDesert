#include "CameraDirector/Assets/CameraStateMachineAsset.h"

const FCameraStateDefinition* UCameraStateMachineAsset::FindStateByName(FName StateName) const
{
	return States.FindByPredicate([StateName](const FCameraStateDefinition& State)
	{
		return State.StateName == StateName;
	});
}

TArray<const FCameraTransitionDefinition*> UCameraStateMachineAsset::GetOutgoingTransitions(FName StateName) const
{
	TArray<const FCameraTransitionDefinition*> Result;

	for (const FCameraTransitionDefinition& Transition : Transitions)
	{
		if (Transition.FromState == StateName)
		{
			Result.Add(&Transition);
		}
	}

	Result.Sort([](const FCameraTransitionDefinition& A, const FCameraTransitionDefinition& B)
	{
		return A.Priority > B.Priority;
	});

	return Result;
}

bool UCameraStateMachineAsset::ValidateAsset(TArray<FString>& OutErrors) const
{
	OutErrors.Reset();

	if (!FindStateByName(EntryState))
	{
		OutErrors.Add(FString::Printf(TEXT("EntryState '%s' was not found in States."), *EntryState.ToString()));
	}

	for (const FCameraStateDefinition& State : States)
	{
		if (State.StateName.IsNone())
		{
			OutErrors.Add(TEXT("Found state with empty StateName."));
		}
	}

	for (const FCameraTransitionDefinition& Transition : Transitions)
	{
		if (!FindStateByName(Transition.FromState))
		{
			OutErrors.Add(FString::Printf(TEXT("Transition has unknown FromState '%s'."), *Transition.FromState.ToString()));
		}

		if (!FindStateByName(Transition.ToState))
		{
			OutErrors.Add(FString::Printf(TEXT("Transition has unknown ToState '%s'."), *Transition.ToState.ToString()));
		}
	}

	return OutErrors.Num() == 0;
}
