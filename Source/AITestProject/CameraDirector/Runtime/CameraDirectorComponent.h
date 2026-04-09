#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CameraDirector/Types/CameraDirectorTypes.h"
#include "CameraDirectorComponent.generated.h"

class UCameraComponent;
class UCameraLogicAsset;
class UCameraLogicPlayer;
class UCameraStateMachineAsset;
class USpringArmComponent;
class FCameraStateMachineRuntime;

DECLARE_LOG_CATEGORY_EXTERN(LogCameraDirector, Log, All);

UCLASS(ClassGroup=(Camera), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class AITESTPROJECT_API UCameraDirectorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCameraDirectorComponent();
	virtual ~UCameraDirectorComponent() override;

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Director")
	TObjectPtr<USpringArmComponent> TargetSpringArm = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Director")
	TObjectPtr<UCameraComponent> TargetCamera = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Director")
	TObjectPtr<UCameraStateMachineAsset> StateMachineAsset = nullptr;

	UFUNCTION(BlueprintCallable, Category = "Camera Director")
	bool PlayCameraLogic(UCameraLogicAsset* Logic);

	UFUNCTION(BlueprintCallable, Category = "Camera Director")
	void StopCameraLogicBySlot(FName SlotName);

	UFUNCTION(BlueprintCallable, Category = "Camera Director")
	void StopAllCameraLogic();

	UFUNCTION(BlueprintCallable, Category = "Camera Director")
	void ForceSetBaseState(FName StateName);

	UFUNCTION(BlueprintPure, Category = "Camera Director")
	FName GetCurrentBaseState() const;

	UFUNCTION(BlueprintPure, Category = "Camera Director")
	FCameraResult GetCurrentCameraResult() const;

	UFUNCTION(BlueprintCallable, Category = "Camera Director|Context")
	void SetLockTarget(AActor* NewLockTarget);

	UFUNCTION(BlueprintCallable, Category = "Camera Director|Context")
	void SetCameraFlags(bool bInIsSprinting, bool bInIsAttacking, bool bInIsHitReact, bool bInWantsLockOn);

protected:
	FCameraStateContext BuildContext(float DeltaTime) const;
	FCameraResult BuildFallbackCameraResult() const;
	void ApplyCameraResult(const FCameraResult& Result);
	void AutoResolveCameraTargets();

	UPROPERTY(Transient)
	TObjectPtr<UCameraLogicPlayer> LogicPlayer = nullptr;

	UPROPERTY(Transient)
	FCameraResult CurrentCameraResult;

	UPROPERTY(Transient)
	TObjectPtr<AActor> LockTargetActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Director|Context")
	bool bIsSprinting = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Director|Context")
	bool bIsAttacking = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Director|Context")
	bool bIsHitReact = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Director|Context")
	bool bWantsLockOn = false;

private:
	FCameraStateMachineRuntime* StateMachineRuntime = nullptr;
};
