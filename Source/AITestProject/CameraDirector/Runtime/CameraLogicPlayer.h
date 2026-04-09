#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CameraDirector/Types/CameraDirectorTypes.h"
#include "CameraLogicPlayer.generated.h"

class UCameraLogicAsset;

USTRUCT()
struct FActiveCameraLogic
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	TObjectPtr<UCameraLogicAsset> LogicInstance = nullptr;

	UPROPERTY(Transient)
	int32 InstanceId = INDEX_NONE;

	UPROPERTY(Transient)
	float BlendElapsed = 0.0f;

	UPROPERTY(Transient)
	float BlendAlpha = 0.0f;

	UPROPERTY(Transient)
	bool bIsStopping = false;
};

UCLASS()
class AITESTPROJECT_API UCameraLogicPlayer : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(UObject* InOwner);

	bool PlayCameraLogic(UCameraLogicAsset* LogicAsset, const FCameraStateContext& Context);
	void StopCameraLogicBySlot(FName SlotName);
	void StopAllCameraLogic();
	void Tick(const FCameraStateContext& Context);
	FCameraResult ApplyCameraLogic(const FCameraStateContext& Context, const FCameraResult& BaseCamera) const;
	bool HasActiveLogic(FName SlotName) const;

private:
	bool CanPlayLogic(const UCameraLogicAsset* LogicAsset) const;
	void RequestStop(FActiveCameraLogic& Entry);
	void CleanupFinished(const FCameraStateContext& Context);
	FActiveCameraLogic* FindOverrideLogic();
	const FActiveCameraLogic* FindOverrideLogic() const;

	UPROPERTY(Transient)
	TObjectPtr<UObject> OwnerObject = nullptr;

	UPROPERTY(Transient)
	TArray<FActiveCameraLogic> ActiveLogics;

	int32 NextInstanceId = 1;
};

