#pragma once

#include "CoreMinimal.h"
#include "CameraDirectorTypes.generated.h"

UENUM(BlueprintType)
enum class ECameraBlendFunction : uint8
{
	Linear,
	EaseIn,
	EaseOut,
	EaseInOut
};

UENUM(BlueprintType)
enum class ECameraStateTransitionRuleType : uint8
{
	AlwaysTrue,
	SpeedGreaterThan,
	SpeedLessThan,
	IsInAir,
	IsGrounded,
	WantsLockOn,
	HasLockTarget,
	BoolFlag
};

UENUM(BlueprintType)
enum class ECameraLogicApplyMode : uint8
{
	Override,
	Additive
};

USTRUCT(BlueprintType)
struct AITESTPROJECT_API FCameraBlendSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend", meta = (ClampMin = "0.0"))
	float BlendTime = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend")
	ECameraBlendFunction BlendFunction = ECameraBlendFunction::EaseInOut;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend", meta = (ClampMin = "0.1"))
	float BlendExponent = 2.0f;

	float EvaluateAlpha(float LinearAlpha) const;
};

USTRUCT(BlueprintType)
struct AITESTPROJECT_API FCameraBasicParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (ClampMin = "0.0"))
	float TargetArmLength = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	FVector SocketOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	FVector TargetOffset = FVector(0.0f, 0.0f, 60.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (ClampMin = "5.0", ClampMax = "170.0"))
	float FOV = 70.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (ClampMin = "0.0"))
	float CameraLagSpeed = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (ClampMin = "0.0"))
	float CameraRotationLagSpeed = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	bool bUsePawnControlRotation = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float YawOffset = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float PitchOffset = 0.0f;
};

USTRUCT(BlueprintType)
struct AITESTPROJECT_API FCameraResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	FVector CameraLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	FRotator CameraRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (ClampMin = "5.0", ClampMax = "170.0"))
	float FOV = 70.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpringArm", meta = (ClampMin = "0.0"))
	float TargetArmLength = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpringArm")
	FVector SocketOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpringArm")
	FVector TargetOffset = FVector(0.0f, 0.0f, 60.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpringArm", meta = (ClampMin = "0.0"))
	float CameraLagSpeed = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpringArm", meta = (ClampMin = "0.0"))
	float CameraRotationLagSpeed = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpringArm")
	bool bUsePawnControlRotation = true;

	static FCameraResult Blend(const FCameraResult& From, const FCameraResult& To, float Alpha);
};

USTRUCT(BlueprintType)
struct AITESTPROJECT_API FCameraStateContext
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Context")
	TObjectPtr<AActor> OwnerActor = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Context")
	TObjectPtr<ACharacter> OwnerCharacter = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Context")
	float DeltaTime = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Context")
	FVector ActorLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "Context")
	FVector Velocity = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "Context")
	FVector Acceleration = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "Context")
	FRotator ControlRotation = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadWrite, Category = "Context")
	float Speed = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Context")
	bool bIsInAir = false;

	UPROPERTY(BlueprintReadWrite, Category = "Context")
	bool bIsAccelerating = false;

	UPROPERTY(BlueprintReadWrite, Category = "Context")
	bool bHasLockTarget = false;

	UPROPERTY(BlueprintReadWrite, Category = "Context")
	TObjectPtr<AActor> LockTargetActor = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Context")
	FVector LockTargetLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "Context")
	bool bIsSprinting = false;

	UPROPERTY(BlueprintReadWrite, Category = "Context")
	bool bIsAttacking = false;

	UPROPERTY(BlueprintReadWrite, Category = "Context")
	bool bIsHitReact = false;

	UPROPERTY(BlueprintReadWrite, Category = "Context")
	bool bWantsLockOn = false;
};

USTRUCT(BlueprintType)
struct AITESTPROJECT_API FCameraStateTransitionRule
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
	ECameraStateTransitionRuleType RuleType = ECameraStateTransitionRuleType::AlwaysTrue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
	float Threshold = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
	FName FlagName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
	bool bExpectedValue = true;

	bool Evaluate(const FCameraStateContext& Context) const;
};

