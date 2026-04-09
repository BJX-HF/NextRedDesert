#include "CameraDirector/Assets/LockOnCameraLogic.h"

ULockOnCameraLogic::ULockOnCameraLogic()
{
	LogicName = TEXT("LockOn");
	ApplyMode = ECameraLogicApplyMode::Override;
	SlotName = TEXT("LockOn");
	Priority = 100;
	BlendIn.BlendTime = 0.2f;
	BlendOut.BlendTime = 0.18f;
}

void ULockOnCameraLogic::OnActivated_Implementation(const FCameraStateContext& Context)
{
	Super::OnActivated_Implementation(Context);

	const FVector OwnerFocus = Context.ActorLocation + FVector(0.0f, 0.0f, HeightOffset);
	const FVector TargetLocation = Context.bHasLockTarget ? Context.LockTargetLocation : OwnerFocus;
	SmoothedTargetFocus = FMath::Lerp(OwnerFocus, TargetLocation, MidpointBias);
	SmoothedRotation = (SmoothedTargetFocus - OwnerFocus).Rotation();
}

void ULockOnCameraLogic::TickLogic_Implementation(const FCameraStateContext& Context)
{
	Super::TickLogic_Implementation(Context);

	if (!Context.bHasLockTarget && bFinishWhenTargetIsLost)
	{
		FinishLogic();
	}
}

void ULockOnCameraLogic::EvaluateCamera_Implementation(const FCameraStateContext& Context, const FCameraResult& BaseCamera, FCameraResult& InOutCamera)
{
	Super::EvaluateCamera_Implementation(Context, BaseCamera, InOutCamera);

	if (!Context.bHasLockTarget)
	{
		return;
	}

	const FVector OwnerFocus = Context.ActorLocation + FVector(0.0f, 0.0f, HeightOffset);
	const FVector TargetFocus = Context.LockTargetLocation + FVector(0.0f, 0.0f, HeightOffset * 0.5f);
	const FVector WeightedMidpoint = FMath::Lerp(OwnerFocus, TargetFocus, MidpointBias);
	const float TargetDistance = FVector::Dist2D(Context.ActorLocation, Context.LockTargetLocation);
	const float DistanceAlpha = FMath::GetMappedRangeValueClamped(FVector2D(150.0f, 1400.0f), FVector2D(0.0f, 1.0f), TargetDistance);

	SmoothedTargetFocus = FMath::VInterpTo(SmoothedTargetFocus, WeightedMidpoint, Context.DeltaTime, PositionInterpSpeed);

	FVector FramingDirection = (Context.ActorLocation - Context.LockTargetLocation).GetSafeNormal2D();
	if (FramingDirection.IsNearlyZero())
	{
		FramingDirection = Context.ControlRotation.Vector().GetSafeNormal2D();
	}

	const FVector RightVector = FVector::CrossProduct(FVector::UpVector, FramingDirection).GetSafeNormal();
	const float DesiredArmLength = FMath::Lerp(MinDistance, MaxDistance, DistanceAlpha);
	const FVector DesiredFocusOffset = (RightVector * LateralOffset) + FVector(0.0f, 0.0f, HeightOffset);
	const FVector DesiredCameraLocation = SmoothedTargetFocus + DesiredFocusOffset + (FramingDirection * DesiredArmLength);
	const FRotator DesiredRotation = (SmoothedTargetFocus - DesiredCameraLocation).Rotation();

	SmoothedRotation = FMath::RInterpTo(SmoothedRotation, DesiredRotation, Context.DeltaTime, RotationInterpSpeed);

	InOutCamera.CameraLocation = FMath::VInterpTo(InOutCamera.CameraLocation, DesiredCameraLocation, Context.DeltaTime, PositionInterpSpeed);
	InOutCamera.CameraRotation = SmoothedRotation;
	InOutCamera.TargetArmLength = DesiredArmLength;
	InOutCamera.TargetOffset = FVector(0.0f, 0.0f, HeightOffset);
	InOutCamera.SocketOffset = FVector(0.0f, LateralOffset, 0.0f);
	InOutCamera.FOV = FMath::Lerp(MinFOV, MaxFOV, DistanceAlpha);
	InOutCamera.bUsePawnControlRotation = true;
}

bool ULockOnCameraLogic::IsFinished_Implementation(const FCameraStateContext& Context) const
{
	return Super::IsFinished_Implementation(Context) || (bFinishWhenTargetIsLost && !Context.bHasLockTarget);
}

