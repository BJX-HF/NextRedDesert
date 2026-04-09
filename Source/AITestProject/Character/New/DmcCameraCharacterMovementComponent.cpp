#include "Character/New/DmcCameraCharacterMovementComponent.h"

#include "Character/New/DmcCameraCharacter.h"
#include "Character/New/WallRunSurfaceComponent.h"
#include "Engine/World.h"

void UDmcCameraCharacterMovementComponent::PhysCustom(float DeltaTime, int32 Iterations)
{
	if (CustomMovementMode == static_cast<uint8>(EDmcCustomMovementMode::CMOVE_WallRun))
	{
		PhysWallRun(DeltaTime, Iterations);
		return;
	}

	Super::PhysCustom(DeltaTime, Iterations);
}

void UDmcCameraCharacterMovementComponent::PhysWallRun(float DeltaTime, int32 Iterations)
{
	if (DeltaTime < MIN_TICK_TIME)
	{
		return;
	}

	ADMCameraCharacter* DmcCharacter = Cast<ADMCameraCharacter>(PawnOwner);
	if (!DmcCharacter || !UpdatedComponent || !DmcCharacter->bIsWallRunning)
	{
		SetMovementMode(MOVE_Falling);
		return;
	}

	const float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;

	UWallRunSurfaceComponent* Surface = DmcCharacter->CurrentWallRunSurface.Get();
	const UWallRunSurfaceComponent* PreviousSurface = Surface;
	if ((!Surface || !DmcCharacter->HasOverlappingWallRunSurface(Surface)) && DmcCharacter->FindBestWallRunSurface())
	{
		Surface = DmcCharacter->FindBestWallRunSurface();
		DmcCharacter->CurrentWallRunSurface = Surface;
	}

	if (Surface && DmcCharacter->HasOverlappingWallRunSurface(Surface) && Surface->IsWallRunEnabled())
	{
		DmcCharacter->CurrentWallNormal = Surface->GetWallNormal();
		DmcCharacter->CurrentWallRunDirection = Surface->GetWallRunDirectionFromVelocity(
			DmcCharacter->CurrentWallRunDirection.IsNearlyZero() ? Velocity : DmcCharacter->CurrentWallRunDirection);
		if (Surface != PreviousSurface || DmcCharacter->CurrentWallRunSide == EWallRunSide::None)
		{
			DmcCharacter->CurrentWallRunSide = DmcCharacter->DetermineWallRunSide(Surface, DmcCharacter->CurrentWallRunDirection);
		}
		DmcCharacter->WallRunLastWallContactTime = CurrentTime;
	}
	else if ((CurrentTime - DmcCharacter->WallRunLastWallContactTime) > DmcCharacter->LostWallForgivenessTime)
	{
		DmcCharacter->StopWallRun(false);
		StartNewPhysics(DeltaTime, Iterations);
		return;
	}

	if ((CurrentTime - DmcCharacter->WallRunStartTime) > DmcCharacter->GetWallRunMaxDurationForCurrentSurface())
	{
		DmcCharacter->StopWallRun(false);
		StartNewPhysics(DeltaTime, Iterations);
		return;
	}

	if (!Acceleration.IsNearlyZero())
	{
		const FVector DesiredFromInput = DmcCharacter->ComputeWallRunDirection(
			DmcCharacter->CurrentWallNormal,
			FVector(Acceleration.X, Acceleration.Y, 0.f));

		DmcCharacter->CurrentWallRunDirection = FMath::VInterpTo(
			DmcCharacter->CurrentWallRunDirection,
			DesiredFromInput,
			DeltaTime,
			6.f).GetSafeNormal();
	}

	const FVector WallRunDirection = DmcCharacter->CurrentWallRunDirection.GetSafeNormal();
	float AlongWallSpeed = FMath::Abs(FVector::DotProduct(Velocity, WallRunDirection));

	const float TargetWallRunSpeed = DmcCharacter->GetWallRunTargetSpeed();
	const float AccelerationInterpSpeed = DmcCharacter->WallRunAcceleration / FMath::Max(TargetWallRunSpeed, 1.f);
	AlongWallSpeed = FMath::FInterpTo(AlongWallSpeed, TargetWallRunSpeed, DeltaTime, AccelerationInterpSpeed);

	if (AlongWallSpeed < DmcCharacter->MinSpeedToMaintainWallRun)
	{
		DmcCharacter->StopWallRun(false);
		StartNewPhysics(DeltaTime, Iterations);
		return;
	}

	const float NewVerticalVelocity = Velocity.Z + GetGravityZ() * DmcCharacter->WallRunGravityScale * DeltaTime;
	const FVector AttractionVelocity = -DmcCharacter->CurrentWallNormal.GetSafeNormal() * DmcCharacter->WallAttractionForce;
	Velocity = (WallRunDirection * AlongWallSpeed) + AttractionVelocity + (FVector::UpVector * NewVerticalVelocity);

	FRotator DesiredRotation = WallRunDirection.Rotation();
	DesiredRotation.Pitch = 0.f;
	DesiredRotation.Roll = 0.f;

	const FQuat TargetQuat = FMath::QInterpTo(
		UpdatedComponent->GetComponentQuat(),
		DesiredRotation.Quaternion(),
		DeltaTime,
		DmcCharacter->WallRunRotationInterpSpeed);

	FHitResult Hit(1.f);
	SafeMoveUpdatedComponent(Velocity * DeltaTime, TargetQuat, true, Hit);

	if (Hit.IsValidBlockingHit())
	{
		if (IsWalkable(Hit))
		{
			DmcCharacter->StopWallRun(false);
			StartNewPhysics(DeltaTime, Iterations);
			return;
		}

		HandleImpact(Hit, DeltaTime, Velocity * DeltaTime);
		SlideAlongSurface(Velocity * DeltaTime, 1.f - Hit.Time, Hit.Normal, Hit, true);
	}
}
