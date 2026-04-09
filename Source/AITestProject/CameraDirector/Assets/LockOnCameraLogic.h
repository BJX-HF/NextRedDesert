#pragma once

#include "CoreMinimal.h"
#include "CameraDirector/Assets/CameraLogicAsset.h"
#include "LockOnCameraLogic.generated.h"

UCLASS(BlueprintType, Blueprintable)
class AITESTPROJECT_API ULockOnCameraLogic : public UCameraLogicAsset
{
	GENERATED_BODY()

public:
	ULockOnCameraLogic();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lock On", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MidpointBias = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lock On", meta = (ClampMin = "0.0"))
	float MinDistance = 300.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lock On", meta = (ClampMin = "0.0"))
	float MaxDistance = 650.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lock On", meta = (ClampMin = "5.0", ClampMax = "170.0"))
	float MinFOV = 58.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lock On", meta = (ClampMin = "5.0", ClampMax = "170.0"))
	float MaxFOV = 72.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lock On")
	float HeightOffset = 65.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lock On")
	float LateralOffset = 24.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lock On", meta = (ClampMin = "0.0"))
	float RecenterSpeed = 5.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lock On", meta = (ClampMin = "0.0"))
	float RotationInterpSpeed = 8.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lock On", meta = (ClampMin = "0.0"))
	float PositionInterpSpeed = 8.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lock On")
	bool bFinishWhenTargetIsLost = true;

	virtual void OnActivated_Implementation(const FCameraStateContext& Context) override;
	virtual void TickLogic_Implementation(const FCameraStateContext& Context) override;
	virtual void EvaluateCamera_Implementation(const FCameraStateContext& Context, const FCameraResult& BaseCamera, FCameraResult& InOutCamera) override;
	virtual bool IsFinished_Implementation(const FCameraStateContext& Context) const override;

protected:
	UPROPERTY(Transient)
	FVector SmoothedTargetFocus = FVector::ZeroVector;

	UPROPERTY(Transient)
	FRotator SmoothedRotation = FRotator::ZeroRotator;
};

