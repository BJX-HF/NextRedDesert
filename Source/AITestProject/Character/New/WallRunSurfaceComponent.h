#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "WallRunSurfaceComponent.generated.h"

UENUM(BlueprintType)
enum class EWallRunNormalAxis : uint8
{
	Forward,
	Backward,
	Right,
	Left
};

UENUM(BlueprintType)
enum class EWallRunAllowedDirection : uint8
{
	Bidirectional,
	PositiveOnly,
	NegativeOnly
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class AITESTPROJECT_API UWallRunSurfaceComponent : public UBoxComponent
{
	GENERATED_BODY()

public:
	UWallRunSurfaceComponent();
	virtual void OnRegister() override;
	virtual void OnComponentCreated() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintPure, Category = "WallRun")
	bool IsWallRunEnabled() const;

	UFUNCTION(BlueprintPure, Category = "WallRun")
	FVector GetWallNormal() const;

	UFUNCTION(BlueprintPure, Category = "WallRun")
	FVector GetPositiveWallRunDirection() const;

	UFUNCTION(BlueprintPure, Category = "WallRun")
	FVector GetWallRunDirectionFromVelocity(const FVector& Velocity) const;

	UFUNCTION(BlueprintPure, Category = "WallRun")
	bool IsDirectionAllowed(const FVector& WallRunDirection) const;

	UFUNCTION(BlueprintPure, Category = "WallRun")
	bool IsPointOnLeftSide(const FVector& CharacterLocation) const;

	UFUNCTION(BlueprintPure, Category = "WallRun")
	FVector GetClosestPointToLocation(const FVector& WorldLocation) const;

	UFUNCTION(BlueprintPure, Category = "WallRun")
	float ResolveWallRunSpeed(float DefaultSpeed) const;

	UFUNCTION(BlueprintPure, Category = "WallRun")
	float ResolveWallRunMaxDuration(float DefaultDuration) const;

protected:
	void ApplyDefaultCollisionSettings();
	void DrawDebugVisualization() const;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallRun")
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallRun")
	EWallRunNormalAxis NormalAxis = EWallRunNormalAxis::Forward;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallRun")
	EWallRunAllowedDirection AllowedDirection = EWallRunAllowedDirection::Bidirectional;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallRun", meta = (ClampMin = 0.0))
	float WallRunSpeedOverride = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallRun", meta = (ClampMin = 0.0))
	float WallRunMaxDurationOverride = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallRun")
	bool bDebugDraw = false;

};
