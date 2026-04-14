#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "Character/New/RDTargetingCameraComponent.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "RDBasecharacter.generated.h"

class UAbilitySystemComponent;
class URDAbilityComponent;
class UAITestAttributeSet;
class UBoxComponent;
class UCameraComponent;
class UGameplayAbility;
class UInputAction;
class UInputMappingContext;
class UPrimitiveComponent;
class USpringArmComponent;
class URDBasecharacterMovementComponent;
class UWallRunSurfaceComponent;
struct FOnAttributeChangeData;

UENUM(BlueprintType)
enum class EWallRunSide : uint8
{
	None,
	Left,
	Right
};

UENUM(BlueprintType)
enum class ERDBasecharacterCustomMovementMode : uint8
{
	CMOVE_None UMETA(DisplayName = "None"),
	CMOVE_WallRun UMETA(DisplayName = "Wall Run")
};

UCLASS()
class AITESTPROJECT_API ARDBasecharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

	friend class URDBasecharacterMovementComponent;

public:
	ARDBasecharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual void Jump() override;



	UFUNCTION(BlueprintCallable, Category = "Movement|Slide")
	bool StartSlide();

	UFUNCTION(BlueprintCallable, Category = "Movement|Slide")
	void StopSlide();

	UFUNCTION(BlueprintPure, Category = "Movement|Slide")
	float GetHorizontalSpeed() const;

	UFUNCTION(BlueprintPure, Category = "Movement|Slide")
	FVector GetSlideDirection() const;

	UFUNCTION(BlueprintPure, Category = "Movement|WallRun")
	bool IsWallRunning() const;

	UFUNCTION(BlueprintPure, Category = "Movement|WallRun")
	bool IsWallJumping() const;

	UFUNCTION(BlueprintPure, Category = "Movement|WallRun")
	bool WasWallJumpRequestedThisFrame() const;

	UFUNCTION(BlueprintPure, Category = "Movement|WallRun")
	bool CanPerformWallJump() const;

	UFUNCTION(BlueprintPure, Category = "Movement|WallRun")
	EWallRunSide GetWallJumpSide() const;

	UFUNCTION(BlueprintPure, Category = "Movement|WallRun")
	bool IsWallJumpFromLeft() const;

	UFUNCTION(BlueprintPure, Category = "Movement|WallRun")
	bool IsWallJumpFromRight() const;

	UFUNCTION(BlueprintPure, Category = "Movement|WallRun")
	EWallRunSide GetWallRunSide() const;

	UFUNCTION(BlueprintPure, Category = "Movement|WallRun")
	bool IsWallRunningOnLeft() const;

	UFUNCTION(BlueprintPure, Category = "Movement|WallRun")
	bool IsWallRunningOnRight() const;

	UFUNCTION(BlueprintPure, Category = "Movement|WallRun")
	FVector GetWallRunDirection() const;

	UFUNCTION(BlueprintPure, Category = "Movement|WallRun")
	FVector GetWallNormal() const;

	UFUNCTION(BlueprintPure, Category = "Movement|WallRun")
	float GetWallRunSpeed() const;

	UFUNCTION(BlueprintPure, Category = "Camera")
	URDTargetingCameraComponent* GetTargetingCameraComponent() const { return TargetingCameraComponent; }

	UFUNCTION(BlueprintPure, Category = "Camera|Target")
	AActor* GetCurrentLockTarget() const;

	UFUNCTION(BlueprintPure, Category = "Camera|Target")
	AActor* GetPreviousLockTarget() const;

	UFUNCTION(BlueprintPure, Category = "Camera|State")
	ECombatCameraState GetCombatCameraState() const;

	UFUNCTION(BlueprintPure, Category = "Camera|State")
	ELockOnZone GetCurrentLockOnZone() const;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void Landed(const FHitResult& Hit) override;
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode = 0) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	TObjectPtr<URDAbilityComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	TObjectPtr<UAITestAttributeSet> AttributeSet;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities")
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities|Attributes", meta = (ClampMin = 0.0))
	float MaxHealth = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities|Attributes")
	float CurrentHealth = 0.0f;

	bool bStartupAbilitiesGranted = false;

	void InitializeAbilitySystem();
	void GrantStartupAbilities();
	void HandleHealthChanged(const FOnAttributeChangeData& ChangeData);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<URDTargetingCameraComponent> TargetingCameraComponent;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|WallRun")
	TObjectPtr<UBoxComponent> WallRunDetector;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LockOnAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> SwitchTargetAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> SlideAction;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slide")
	bool bSlideEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slide")
	bool bCanSlideFromStandingStill = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slide", meta = (ClampMin = 0.0))
	float SlideTriggerMinSpeed = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slide", meta = (ClampMin = 0.0))
	float SlideMinStartSpeed = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slide", meta = (ClampMin = 0.0))
	float SlideInitialSpeedBoost = 180.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slide", meta = (ClampMin = 0.0))
	float SlideMaxSpeed = 900.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slide", meta = (ClampMin = 0.0))
	float SlideAcceleration = 1200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slide", meta = (ClampMin = 0.0))
	float SlideBrakingDeceleration = 450.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slide", meta = (ClampMin = 0.0))
	float SlideGroundFriction = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slide", meta = (ClampMin = 0.0))
	float SlideDuration = 0.85f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slide", meta = (ClampMin = 0.0))
	float SlideMinSpeedToEnd = 180.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slide", meta = (ClampMin = 0.0, ClampMax = 1.0))
	float SlideSteeringControl = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slide", meta = (ClampMin = 0.0))
	float SlideCooldown = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slide", meta = (ClampMin = 0.0))
	float SlideCapsuleHalfHeight = 48.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Slide")
	bool bIsSliding = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Slide")
	float SlideTimeRemaining = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Slide")
	float SlideCooldownRemaining = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Slide")
	FVector SlideDirection = FVector::ForwardVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Animation")
	float HorizontalSpeed = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Animation")
	bool bHasSlideInputQueued = false;

	float DefaultCapsuleHalfHeight = 0.f;
	float DefaultGroundFriction = 0.f;
	float DefaultBrakingDecelerationWalking = 0.f;
	float DefaultMaxWalkSpeed = 0.f;
	bool bPendingSlideUnCrouch = false;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|WallRun|Debug")
	bool bDebugWallRun = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|WallRun", meta = (ClampMin = 0.0))
	float MinSpeedToStartWallRun = 550.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|WallRun", meta = (ClampMin = 0.0))
	float MinSpeedToMaintainWallRun = 400.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|WallRun", meta = (ClampMin = 0.0, ClampMax = 180.0))
	float MinWallAngleFromUpDeg = 65.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|WallRun", meta = (ClampMin = 0.0, ClampMax = 180.0))
	float MaxWallAngleFromUpDeg = 115.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|WallRun", meta = (ClampMin = 0.0, ClampMax = 180.0))
	float MinApproachAngleDeg = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|WallRun", meta = (ClampMin = 0.0, ClampMax = 180.0))
	float MaxApproachAngleDeg = 75.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|WallRun", meta = (ClampMin = 0.0))
	float JumpToWallGraceTime = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|WallRun", meta = (ClampMin = 0.0))
	float WallRunSpeed = 900.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|WallRun", meta = (ClampMin = 0.0))
	float WallRunAcceleration = 2200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|WallRun", meta = (ClampMin = 0.0))
	float WallRunGravityScale = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|WallRun", meta = (ClampMin = 0.0))
	float WallAttractionForce = 350.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|WallRun", meta = (ClampMin = 0.0))
	float WallRunMaxDuration = 1.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|WallRun", meta = (ClampMin = 0.0))
	float LostWallForgivenessTime = 0.12f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|WallRun|WallJump")
	bool bEnableWallJump = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|WallRun|WallJump", meta = (ClampMin = 0.0))
	float WallJumpHorizontalStrength = 520.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|WallRun|WallJump", meta = (ClampMin = 0.0))
	float WallJumpVerticalStrength = 650.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|WallRun|WallJump", meta = (ClampMin = 0.0))
	float WallJumpForwardBoost = 140.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|WallRun", meta = (ClampMin = 0.0))
	float WallRunRotationInterpSpeed = 12.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|WallRun|WallJump", meta = (ClampMin = 0.0))
	float WallJumpAnimFlagDuration = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|WallRun|WallJump", meta = (ClampMin = 0.0))
	float WallJumpReattachBlockTime = 0.25f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|WallRun")
	bool bIsWallRunning = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|WallRun")
	bool bWallJumpTriggered = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|WallRun")
	bool bWallJumpRequested = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|WallRun")
	FVector CurrentWallNormal = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|WallRun")
	FVector CurrentWallRunDirection = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|WallRun")
	EWallRunSide CurrentWallRunSide = EWallRunSide::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|WallRun")
	EWallRunSide LastWallJumpSide = EWallRunSide::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|WallRun")
	float WallRunStartTime = -1000.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|WallRun")
	float LastJumpPressedTime = -1000.f;

	TWeakObjectPtr<UWallRunSurfaceComponent> CurrentWallRunSurface;
	TWeakObjectPtr<UWallRunSurfaceComponent> LastWallJumpSurface;
	TArray<TWeakObjectPtr<UWallRunSurfaceComponent>> OverlappingWallRunSurfaces;
	float WallRunLastWallContactTime = -1000.f;
	float WallJumpAnimFlagRemaining = 0.f;
	float WallJumpBlockedUntilTime = -1000.f;
	mutable float LastWallRunDebugTime = -1000.f;

protected:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void StartJump(const FInputActionValue& Value);
	void StopJump(const FInputActionValue& Value);
	void HandleSlideInput(const FInputActionValue& Value);
	void ToggleLockOn(const FInputActionValue& Value);
	void SwitchTarget(const FInputActionValue& Value);

	void UpdateWallRunState(float DeltaSeconds);
	bool TryStartWallRun();
	bool CanStartWallRunOnSurface(UWallRunSurfaceComponent* Surface) const;
	UWallRunSurfaceComponent* FindBestWallRunSurface() const;
	void StartWallRun(UWallRunSurfaceComponent* Surface);
	void StopWallRun(bool bLaunchAway);
	void PerformWallJump();

	UFUNCTION()
	void OnWallRunDetectorBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void OnWallRunDetectorEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	void AddOverlappingWallRunSurface(UWallRunSurfaceComponent* Surface);
	void RemoveOverlappingWallRunSurface(UWallRunSurfaceComponent* Surface);
	bool HasOverlappingWallRunSurface(UWallRunSurfaceComponent* Surface) const;
	void CleanupInvalidWallRunSurfaces();

	FVector ComputeWallRunDirection(const FVector& WallNormal, const FVector& ReferenceDirection) const;
	EWallRunSide DetermineWallRunSide(const UWallRunSurfaceComponent* Surface, const FVector& WallRunDirection) const;
	float GetWallAngleFromUpDeg(const FVector& WallNormal) const;
	float GetWallApproachAngleDeg(const FVector& WallNormal) const;
	float GetWallRunTargetSpeed() const;
	float GetWallRunMaxDurationForCurrentSurface() const;
	bool IsWallRunMovementMode() const;
	void ClearWallRunState();
	void SetWallJumpTriggered(bool bNewTriggered);
	void DebugWallRunMessage(const FString& Message, const FColor& Color = FColor::Yellow) const;
	URDBasecharacterMovementComponent* GetRDBasecharacterMovementComponent() const;

	void UpdateCameraSystem(float DeltaSeconds);
	void UpdateSlide(float DeltaSeconds);
	void UpdateCharacterFacing(float DeltaSeconds);

	bool CanStartSlide() const;
	float GetClampedSlideCapsuleHalfHeight() const;
};
