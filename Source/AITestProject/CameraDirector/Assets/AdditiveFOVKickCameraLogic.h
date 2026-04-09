#pragma once

#include "CoreMinimal.h"
#include "Curves/CurveFloat.h"
#include "CameraDirector/Assets/CameraLogicAsset.h"
#include "AdditiveFOVKickCameraLogic.generated.h"

UCLASS(BlueprintType, Blueprintable)
class AITESTPROJECT_API UAdditiveFOVKickCameraLogic : public UCameraLogicAsset
{
	GENERATED_BODY()

public:
	UAdditiveFOVKickCameraLogic();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FOV Kick")
	float FOVKickAmount = 8.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FOV Kick", meta = (ClampMin = "0.01"))
	float Duration = 0.25f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FOV Kick")
	TObjectPtr<UCurveFloat> IntensityCurve = nullptr;

	virtual void TickLogic_Implementation(const FCameraStateContext& Context) override;
	virtual void EvaluateCamera_Implementation(const FCameraStateContext& Context, const FCameraResult& BaseCamera, FCameraResult& InOutCamera) override;
	virtual bool IsFinished_Implementation(const FCameraStateContext& Context) const override;
};

