#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CameraDirector/Types/CameraDirectorTypes.h"
#include "CameraLogicAsset.generated.h"

UCLASS(Abstract, BlueprintType, Blueprintable)
class AITESTPROJECT_API UCameraLogicAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera Logic")
	FName LogicName = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera Logic")
	ECameraLogicApplyMode ApplyMode = ECameraLogicApplyMode::Override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera Logic")
	FName SlotName = TEXT("Default");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera Logic")
	int32 Priority = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera Logic")
	FCameraBlendSettings BlendIn;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera Logic")
	FCameraBlendSettings BlendOut;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera Logic")
	bool bAutoFinish = false;

	UFUNCTION(BlueprintNativeEvent, Category = "Camera Logic")
	void OnActivated(const FCameraStateContext& Context);

	UFUNCTION(BlueprintNativeEvent, Category = "Camera Logic")
	void OnDeactivated(const FCameraStateContext& Context);

	UFUNCTION(BlueprintNativeEvent, Category = "Camera Logic")
	void TickLogic(const FCameraStateContext& Context);

	UFUNCTION(BlueprintNativeEvent, Category = "Camera Logic")
	void EvaluateCamera(const FCameraStateContext& Context, const FCameraResult& BaseCamera, UPARAM(ref) FCameraResult& InOutCamera);

	UFUNCTION(BlueprintNativeEvent, BlueprintPure = false, Category = "Camera Logic")
	bool IsFinished(const FCameraStateContext& Context) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintPure = false, Category = "Camera Logic")
	bool CanBeInterruptedBy(const UCameraLogicAsset* OtherLogic) const;

	UFUNCTION(BlueprintCallable, Category = "Camera Logic")
	void FinishLogic();

	UFUNCTION(BlueprintPure, Category = "Camera Logic")
	float GetRuntimeElapsedTime() const;

	UFUNCTION(BlueprintPure, Category = "Camera Logic")
	bool IsRuntimeStopping() const;

	void ResetRuntimeState();
	void AdvanceRuntime(float DeltaTime);
	void SetRuntimeStopping(bool bInStopping);

protected:
	UPROPERTY(Transient)
	float RuntimeElapsedTime = 0.0f;

	UPROPERTY(Transient)
	bool bRuntimeRequestedFinish = false;

	UPROPERTY(Transient)
	bool bRuntimeStopping = false;
};
