#pragma once

#include "CoreMinimal.h"
#include "CameraDirector/Types/CameraDirectorTypes.h"

class UCameraStateMachineAsset;
struct FCameraStateDefinition;
struct FCameraTransitionDefinition;

class AITESTPROJECT_API FCameraStateMachineRuntime
{
public:
	void Initialize(UCameraStateMachineAsset* InAsset);
	void ForceSetState(FName StateName);
	void Tick(const FCameraStateContext& Context);

	FName GetCurrentStateName() const { return CurrentStateName; }
	FName GetPreviousStateName() const { return PreviousStateName; }
	FCameraResult GetCurrentResult() const { return CurrentResult; }

private:
	void StartTransition(const FCameraTransitionDefinition& Transition);
	FCameraResult BuildCameraResultFromState(const FCameraStateDefinition& State, const FCameraStateContext& Context) const;
	const FCameraTransitionDefinition* FindBestTransition(const FCameraStateContext& Context) const;

	TObjectPtr<UCameraStateMachineAsset> Asset = nullptr;
	FName CurrentStateName = NAME_None;
	FName PreviousStateName = NAME_None;
	FCameraBlendSettings ActiveBlendSettings;
	float BlendElapsed = 0.0f;
	bool bIsBlending = false;
	FCameraResult CurrentResult;
};

