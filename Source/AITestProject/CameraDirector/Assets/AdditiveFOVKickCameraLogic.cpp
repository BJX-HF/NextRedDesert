#include "CameraDirector/Assets/AdditiveFOVKickCameraLogic.h"

UAdditiveFOVKickCameraLogic::UAdditiveFOVKickCameraLogic()
{
	LogicName = TEXT("FOVKick");
	ApplyMode = ECameraLogicApplyMode::Additive;
	SlotName = TEXT("Reaction");
	Priority = 10;
	bAutoFinish = true;
	BlendIn.BlendTime = 0.03f;
	BlendOut.BlendTime = 0.12f;
}

void UAdditiveFOVKickCameraLogic::TickLogic_Implementation(const FCameraStateContext& Context)
{
	Super::TickLogic_Implementation(Context);

	if (GetRuntimeElapsedTime() >= Duration)
	{
		FinishLogic();
	}
}

void UAdditiveFOVKickCameraLogic::EvaluateCamera_Implementation(const FCameraStateContext& Context, const FCameraResult& BaseCamera, FCameraResult& InOutCamera)
{
	Super::EvaluateCamera_Implementation(Context, BaseCamera, InOutCamera);

	const float SafeDuration = FMath::Max(Duration, KINDA_SMALL_NUMBER);
	const float NormalizedTime = FMath::Clamp(GetRuntimeElapsedTime() / SafeDuration, 0.0f, 1.0f);
	const float Intensity = IntensityCurve ? IntensityCurve->GetFloatValue(NormalizedTime) : (1.0f - NormalizedTime);

	InOutCamera.FOV += FOVKickAmount * Intensity;
}

bool UAdditiveFOVKickCameraLogic::IsFinished_Implementation(const FCameraStateContext& Context) const
{
	return Super::IsFinished_Implementation(Context) || GetRuntimeElapsedTime() >= Duration;
}
