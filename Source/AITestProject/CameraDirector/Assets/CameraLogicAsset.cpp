#include "CameraDirector/Assets/CameraLogicAsset.h"

void UCameraLogicAsset::OnActivated_Implementation(const FCameraStateContext& Context)
{
}

void UCameraLogicAsset::OnDeactivated_Implementation(const FCameraStateContext& Context)
{
}

void UCameraLogicAsset::TickLogic_Implementation(const FCameraStateContext& Context)
{
}

void UCameraLogicAsset::EvaluateCamera_Implementation(const FCameraStateContext& Context, const FCameraResult& BaseCamera, FCameraResult& InOutCamera)
{
}

bool UCameraLogicAsset::IsFinished_Implementation(const FCameraStateContext& Context) const
{
	return bAutoFinish && bRuntimeRequestedFinish;
}

bool UCameraLogicAsset::CanBeInterruptedBy_Implementation(const UCameraLogicAsset* OtherLogic) const
{
	return true;
}

void UCameraLogicAsset::FinishLogic()
{
	bRuntimeRequestedFinish = true;
}

float UCameraLogicAsset::GetRuntimeElapsedTime() const
{
	return RuntimeElapsedTime;
}

bool UCameraLogicAsset::IsRuntimeStopping() const
{
	return bRuntimeStopping;
}

void UCameraLogicAsset::ResetRuntimeState()
{
	RuntimeElapsedTime = 0.0f;
	bRuntimeRequestedFinish = false;
	bRuntimeStopping = false;
}

void UCameraLogicAsset::AdvanceRuntime(float DeltaTime)
{
	RuntimeElapsedTime += DeltaTime;
}

void UCameraLogicAsset::SetRuntimeStopping(bool bInStopping)
{
	bRuntimeStopping = bInStopping;
}

