#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RDTargetingCameraComponent.generated.h"

class ACharacter;
class UCameraComponent;
class USpringArmComponent;

UENUM(BlueprintType)
enum class ECombatCameraState : uint8
{
	Explore,
	CombatFree,
	LockOn,
	Recover
};

UENUM(BlueprintType)
enum class ELockOnZone : uint8
{
	Center,
	Buffer,
	Edge,
	Offscreen
};

UCLASS(ClassGroup=(Camera), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class AITESTPROJECT_API URDTargetingCameraComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URDTargetingCameraComponent();

	void InitializeCamera(USpringArmComponent* InCameraBoom, UCameraComponent* InFollowCamera);
	void TickCamera(float DeltaSeconds);
	void HandleLookInput(const FVector2D& LookAxisVector);
	void ToggleLockOn();
	void SwitchTarget(float AxisValue);

	UFUNCTION(BlueprintPure, Category = "Camera|State")
	ECombatCameraState GetCameraState() const { return CameraState; }

	UFUNCTION(BlueprintPure, Category = "Camera|State")
	ELockOnZone GetCurrentZone() const { return CurrentZone; }

	UFUNCTION(BlueprintPure, Category = "Camera|Target")
	AActor* GetCurrentLockTarget() const { return CurrentLockTarget; }

	UFUNCTION(BlueprintPure, Category = "Camera|Target")
	AActor* GetPreviousLockTarget() const { return PreviousLockTarget; }

	FVector GetTargetPivot(AActor* Target) const;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(Transient)
	TObjectPtr<ACharacter> OwnerCharacter = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<USpringArmComponent> CameraBoom = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UCameraComponent> FollowCamera = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera|State")
	ECombatCameraState CameraState = ECombatCameraState::Explore;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera|State")
	ELockOnZone CurrentZone = ELockOnZone::Center;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera|Target")
	TObjectPtr<AActor> CurrentLockTarget = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera|Target")
	TObjectPtr<AActor> PreviousLockTarget = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|Detect")
	float CombatDetectRadius = 1800.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|Detect")
	float LockAcquireRadius = 2200.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|Detect")
	float MaxLockVerticalDelta = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|Detect")
	FName LockTargetTag = TEXT("LockTarget");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|State")
	float UnlockRecoveryDelay = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera|State")
	float UnlockRecoveryRemaining = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|Explore")
	float ExploreArmLength = 420.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|Explore")
	FVector ExploreSocketOffset = FVector(0.f, 50.f, 65.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|Explore")
	float ExploreFOV = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|Explore")
	float ExploreMinPitch = -50.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|Explore")
	float ExploreMaxPitch = 25.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|CombatFree")
	float CombatArmLength = 390.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|CombatFree")
	FVector CombatSocketOffset = FVector(0.f, 35.f, 70.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|CombatFree")
	float CombatFOV = 58.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|CombatFree")
	float CombatMinPitch = -25.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|CombatFree")
	float CombatMaxPitch = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|LockOn")
	float NearDistance = 350.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|LockOn")
	float MidDistance = 800.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|LockOn")
	float LockNearArmLength = 430.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|LockOn")
	float LockMidArmLength = 380.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|LockOn")
	float LockFarArmLength = 360.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|LockOn")
	FVector LockNearSocketOffset = FVector(0.f, 20.f, 80.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|LockOn")
	FVector LockMidSocketOffset = FVector(0.f, 15.f, 75.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|LockOn")
	FVector LockFarSocketOffset = FVector(0.f, 10.f, 70.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|LockOn")
	float LockNearFOV = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|LockOn")
	float LockMidFOV = 57.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|LockOn")
	float LockFarFOV = 55.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|LockOn")
	float LockMinPitch = -20.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|LockOn")
	float LockMaxPitch = 18.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|LockOn")
	float CenterAngle = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|LockOn")
	float BufferAngle = 35.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|LockOn")
	float EdgeAngle = 65.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|Interp")
	float ArmInterpSpeed = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|Interp")
	float OffsetInterpSpeed = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|Interp")
	float RotationInterpSpeed_Explore = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|Interp")
	float RotationInterpSpeed_Combat = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|Interp")
	float RotationInterpSpeed_Lock_Center = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|Interp")
	float RotationInterpSpeed_Lock_Buffer = 7.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|Interp")
	float RotationInterpSpeed_Lock_Edge = 11.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|Interp")
	float RotationInterpSpeed_Lock_Offscreen = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|Interp")
	float FOVInterpSpeed = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|Orbit")
	float LockOrbitYawMax = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|Orbit")
	float LockOrbitPitchMax = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|Orbit")
	float LockOrbitYawInputScale = 2.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|Orbit")
	float LockOrbitPitchInputScale = 1.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|Orbit")
	float LockOrbitDecaySpeed = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|Orbit")
	float HeightPitchFactor = 0.03f;

	float DesiredArmLength = 420.f;
	FVector DesiredSocketOffset = FVector::ZeroVector;
	FRotator DesiredControlRotation = FRotator::ZeroRotator;
	float DesiredFOV = 60.f;

	float PendingOrbitYawInput = 0.f;
	float PendingOrbitPitchInput = 0.f;
	float LockOrbitYaw = 0.f;
	float LockOrbitPitch = 0.f;
	float TargetSwitchCooldown = 0.f;

	void UpdateCameraState(float DeltaSeconds);
	void SolveDesiredCamera(float DeltaSeconds);
	void ApplyCamera(float DeltaSeconds);

	bool HasNearbyCombatTarget() const;
	bool IsValidLockTarget(AActor* Actor) const;
	TArray<AActor*> GatherCandidateTargets(float Radius) const;
	AActor* FindBestLockTarget() const;
	AActor* FindSwitchTarget(float DirectionSign) const;

	ELockOnZone ComputeLockZone(float TargetYawDeg) const;
	float GetRotationInterpSpeedForZone(ELockOnZone Zone) const;

	FVector GetPlayerPivot() const;
	float GetDistanceToTarget2D(AActor* Target) const;
	bool IsTargetInFrontHemisphere(AActor* Target) const;

	void EnterLockOn(AActor* NewTarget);
	void ExitLockOn();
};
