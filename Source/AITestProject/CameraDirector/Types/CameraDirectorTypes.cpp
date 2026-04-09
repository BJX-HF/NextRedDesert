#include "CameraDirector/Types/CameraDirectorTypes.h"

namespace CameraDirector
{
	static float ApplyBlendFunction(float Alpha, ECameraBlendFunction Function, float Exponent)
	{
		switch (Function)
		{
		case ECameraBlendFunction::Linear:
			return Alpha;
		case ECameraBlendFunction::EaseIn:
			return FMath::InterpEaseIn(0.0f, 1.0f, Alpha, Exponent);
		case ECameraBlendFunction::EaseOut:
			return FMath::InterpEaseOut(0.0f, 1.0f, Alpha, Exponent);
		case ECameraBlendFunction::EaseInOut:
		default:
			return FMath::InterpEaseInOut(0.0f, 1.0f, Alpha, Exponent);
		}
	}

	static bool EvaluateNamedBoolFlag(const FCameraStateContext& Context, const FName FlagName)
	{
		static const FName NAME_IsSprinting(TEXT("IsSprinting"));
		static const FName NAME_IsAttacking(TEXT("IsAttacking"));
		static const FName NAME_IsHitReact(TEXT("IsHitReact"));
		static const FName NAME_IsAccelerating(TEXT("IsAccelerating"));
		static const FName NAME_IsInAir(TEXT("IsInAir"));
		static const FName NAME_WantsLockOn(TEXT("WantsLockOn"));
		static const FName NAME_HasLockTarget(TEXT("HasLockTarget"));

		if (FlagName == NAME_IsSprinting) return Context.bIsSprinting;
		if (FlagName == NAME_IsAttacking) return Context.bIsAttacking;
		if (FlagName == NAME_IsHitReact) return Context.bIsHitReact;
		if (FlagName == NAME_IsAccelerating) return Context.bIsAccelerating;
		if (FlagName == NAME_IsInAir) return Context.bIsInAir;
		if (FlagName == NAME_WantsLockOn) return Context.bWantsLockOn;
		if (FlagName == NAME_HasLockTarget) return Context.bHasLockTarget;
		return false;
	}
}

float FCameraBlendSettings::EvaluateAlpha(float LinearAlpha) const
{
	return CameraDirector::ApplyBlendFunction(FMath::Clamp(LinearAlpha, 0.0f, 1.0f), BlendFunction, BlendExponent);
}

FCameraResult FCameraResult::Blend(const FCameraResult& From, const FCameraResult& To, float Alpha)
{
	const float ClampedAlpha = FMath::Clamp(Alpha, 0.0f, 1.0f);

	FCameraResult Result;
	Result.CameraLocation = FMath::Lerp(From.CameraLocation, To.CameraLocation, ClampedAlpha);
	Result.CameraRotation = FQuat::Slerp(From.CameraRotation.Quaternion(), To.CameraRotation.Quaternion(), ClampedAlpha).Rotator();
	Result.FOV = FMath::Lerp(From.FOV, To.FOV, ClampedAlpha);
	Result.TargetArmLength = FMath::Lerp(From.TargetArmLength, To.TargetArmLength, ClampedAlpha);
	Result.SocketOffset = FMath::Lerp(From.SocketOffset, To.SocketOffset, ClampedAlpha);
	Result.TargetOffset = FMath::Lerp(From.TargetOffset, To.TargetOffset, ClampedAlpha);
	Result.CameraLagSpeed = FMath::Lerp(From.CameraLagSpeed, To.CameraLagSpeed, ClampedAlpha);
	Result.CameraRotationLagSpeed = FMath::Lerp(From.CameraRotationLagSpeed, To.CameraRotationLagSpeed, ClampedAlpha);
	Result.bUsePawnControlRotation = ClampedAlpha < 0.5f ? From.bUsePawnControlRotation : To.bUsePawnControlRotation;
	return Result;
}

bool FCameraStateTransitionRule::Evaluate(const FCameraStateContext& Context) const
{
	switch (RuleType)
	{
	case ECameraStateTransitionRuleType::AlwaysTrue:
		return true;
	case ECameraStateTransitionRuleType::SpeedGreaterThan:
		return Context.Speed > Threshold;
	case ECameraStateTransitionRuleType::SpeedLessThan:
		return Context.Speed < Threshold;
	case ECameraStateTransitionRuleType::IsInAir:
		return Context.bIsInAir;
	case ECameraStateTransitionRuleType::IsGrounded:
		return !Context.bIsInAir;
	case ECameraStateTransitionRuleType::WantsLockOn:
		return Context.bWantsLockOn;
	case ECameraStateTransitionRuleType::HasLockTarget:
		return Context.bHasLockTarget;
	case ECameraStateTransitionRuleType::BoolFlag:
		return CameraDirector::EvaluateNamedBoolFlag(Context, FlagName) == bExpectedValue;
	default:
		return false;
	}
}
