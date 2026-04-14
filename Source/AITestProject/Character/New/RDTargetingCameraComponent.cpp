#include "Character/New/RDTargetingCameraComponent.h"

#include "Camera/CameraComponent.h"
#include "CollisionShape.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "WorldCollision.h"

URDTargetingCameraComponent::URDTargetingCameraComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	DesiredArmLength = ExploreArmLength;
	DesiredSocketOffset = ExploreSocketOffset;
	DesiredFOV = ExploreFOV;
}

void URDTargetingCameraComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter)
	{
		if (APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController()))
		{
			DesiredControlRotation = PC->GetControlRotation();
		}
	}
}

void URDTargetingCameraComponent::InitializeCamera(USpringArmComponent* InCameraBoom, UCameraComponent* InFollowCamera)
{
	CameraBoom = InCameraBoom;
	FollowCamera = InFollowCamera;
}

void URDTargetingCameraComponent::TickCamera(float DeltaSeconds)
{
	OwnerCharacter = OwnerCharacter ? OwnerCharacter.Get() : Cast<ACharacter>(GetOwner());

	if (TargetSwitchCooldown > 0.f)
	{
		TargetSwitchCooldown -= DeltaSeconds;
	}

	UpdateCameraState(DeltaSeconds);
	SolveDesiredCamera(DeltaSeconds);
	ApplyCamera(DeltaSeconds);
}

void URDTargetingCameraComponent::HandleLookInput(const FVector2D& LookAxisVector)
{
	if (!OwnerCharacter)
	{
		OwnerCharacter = Cast<ACharacter>(GetOwner());
	}

	if (!OwnerCharacter || !OwnerCharacter->GetController())
	{
		return;
	}

	if (CameraState == ECombatCameraState::LockOn)
	{
		return;
	}

	OwnerCharacter->AddControllerYawInput(LookAxisVector.X);
	OwnerCharacter->AddControllerPitchInput(LookAxisVector.Y);

	if (APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController()))
	{
		FRotator ControlRot = PC->GetControlRotation();
		const float MinPitch = (CameraState == ECombatCameraState::Explore) ? ExploreMinPitch : CombatMinPitch;
		const float MaxPitch = (CameraState == ECombatCameraState::Explore) ? ExploreMaxPitch : CombatMaxPitch;
		ControlRot.Pitch = FMath::ClampAngle(ControlRot.Pitch, MinPitch, MaxPitch);
		PC->SetControlRotation(ControlRot);
	}
}

void URDTargetingCameraComponent::ToggleLockOn()
{
	if (CurrentLockTarget)
	{
		ExitLockOn();
		return;
	}

	if (AActor* NewTarget = FindBestLockTarget())
	{
		EnterLockOn(NewTarget);
	}
}

void URDTargetingCameraComponent::SwitchTarget(float AxisValue)
{
	if (!CurrentLockTarget || CameraState != ECombatCameraState::LockOn || TargetSwitchCooldown > 0.f)
	{
		return;
	}

	if (FMath::Abs(AxisValue) < 0.5f)
	{
		return;
	}

	if (AActor* NewTarget = FindSwitchTarget(FMath::Sign(AxisValue)))
	{
		if (NewTarget != CurrentLockTarget)
		{
			PreviousLockTarget = CurrentLockTarget;
			CurrentLockTarget = NewTarget;
			TargetSwitchCooldown = 0.2f;
		}
	}
}

void URDTargetingCameraComponent::UpdateCameraState(float DeltaSeconds)
{
	if (CurrentLockTarget && !IsValidLockTarget(CurrentLockTarget))
	{
		ExitLockOn();
	}

	if (CurrentLockTarget)
	{
		CameraState = ECombatCameraState::LockOn;
		return;
	}

	if (UnlockRecoveryRemaining > 0.f)
	{
		UnlockRecoveryRemaining -= DeltaSeconds;
		CameraState = ECombatCameraState::Recover;
		return;
	}

	CameraState = HasNearbyCombatTarget() ? ECombatCameraState::CombatFree : ECombatCameraState::Explore;
}

void URDTargetingCameraComponent::SolveDesiredCamera(float DeltaSeconds)
{
	if (!OwnerCharacter)
	{
		OwnerCharacter = Cast<ACharacter>(GetOwner());
	}

	APlayerController* PC = OwnerCharacter ? Cast<APlayerController>(OwnerCharacter->GetController()) : nullptr;
	if (!PC) return;

	FRotator CurrentControlRot = PC->GetControlRotation();

	if (CameraState == ECombatCameraState::Explore)
	{
		DesiredArmLength = ExploreArmLength;
		DesiredSocketOffset = ExploreSocketOffset;
		DesiredFOV = ExploreFOV;
		DesiredControlRotation = CurrentControlRot;
		DesiredControlRotation.Pitch = FMath::ClampAngle(DesiredControlRotation.Pitch, ExploreMinPitch, ExploreMaxPitch);
		CurrentZone = ELockOnZone::Center;
		return;
	}

	if (CameraState == ECombatCameraState::CombatFree || CameraState == ECombatCameraState::Recover)
	{
		DesiredArmLength = CombatArmLength;
		DesiredSocketOffset = CombatSocketOffset;
		DesiredFOV = CombatFOV;
		DesiredControlRotation = CurrentControlRot;
		DesiredControlRotation.Pitch = FMath::ClampAngle(DesiredControlRotation.Pitch, CombatMinPitch, CombatMaxPitch);
		CurrentZone = ELockOnZone::Center;
		return;
	}

	if (!CurrentLockTarget)
	{
		return;
	}

	const FVector PlayerPivot = GetPlayerPivot();
	const FVector TargetPivot = GetTargetPivot(CurrentLockTarget);
	const FVector ToTarget = TargetPivot - PlayerPivot;
	const float Distance2D = FVector(ToTarget.X, ToTarget.Y, 0.f).Length();

	if (Distance2D <= NearDistance)
	{
		DesiredArmLength = LockNearArmLength;
		DesiredSocketOffset = LockNearSocketOffset;
		DesiredFOV = LockNearFOV;
	}
	else if (Distance2D <= MidDistance)
	{
		DesiredArmLength = LockMidArmLength;
		DesiredSocketOffset = LockMidSocketOffset;
		DesiredFOV = LockMidFOV;
	}
	else
	{
		DesiredArmLength = LockFarArmLength;
		DesiredSocketOffset = LockFarSocketOffset;
		DesiredFOV = LockFarFOV;
	}

	const float HeightDelta = TargetPivot.Z - PlayerPivot.Z;
	LockOrbitYaw = FMath::Clamp(LockOrbitYaw + PendingOrbitYawInput, -LockOrbitYawMax, LockOrbitYawMax);
	LockOrbitPitch = FMath::Clamp(LockOrbitPitch + PendingOrbitPitchInput, -LockOrbitPitchMax, LockOrbitPitchMax);
	PendingOrbitYawInput = 0.f;
	PendingOrbitPitchInput = 0.f;

	const FRotator BaseLookRot = ToTarget.Rotation();
	float DesiredYaw = BaseLookRot.Yaw + LockOrbitYaw;
	float DesiredPitch = BaseLookRot.Pitch + HeightDelta * HeightPitchFactor + LockOrbitPitch;
	DesiredPitch = FMath::ClampAngle(DesiredPitch, LockMinPitch, LockMaxPitch);

	CurrentLockDeltaYaw = FMath::Abs(FMath::FindDeltaAngleDegrees(CurrentControlRot.Yaw, BaseLookRot.Yaw));
	CurrentZone = ComputeLockZone(BaseLookRot.Yaw);

	if (RotationInterpSpeed_Lock_Center <= 0.f)
	{
		const float StartRotationAngle = CenterAngle + LockRotationHysteresisAngle;
		const float StopRotationAngle = FMath::Max(0.f, CenterAngle - LockRotationHysteresisAngle);

		if (bLockCameraRotationActive)
		{
			bLockCameraRotationActive = CurrentLockDeltaYaw > StopRotationAngle;
		}
		else
		{
			bLockCameraRotationActive = CurrentLockDeltaYaw > StartRotationAngle;
		}
	}
	else
	{
		bLockCameraRotationActive = true;
	}

	DrawLockAngleDebug(BaseLookRot.Yaw);

	const bool bDangerZone = CurrentZone == ELockOnZone::Edge || CurrentZone == ELockOnZone::Offscreen;
	LockOrbitYaw = FMath::FInterpTo(LockOrbitYaw, 0.f, DeltaSeconds, bDangerZone ? LockOrbitDecaySpeed * 2.0f : LockOrbitDecaySpeed);
	LockOrbitPitch = FMath::FInterpTo(LockOrbitPitch, 0.f, DeltaSeconds, LockOrbitDecaySpeed);

	if (CurrentZone == ELockOnZone::Offscreen)
	{
		DesiredArmLength += 30.f;
		DesiredSocketOffset.Z += 10.f;
	}

	DesiredControlRotation = FRotator(DesiredPitch, DesiredYaw, 0.f);
}

void URDTargetingCameraComponent::ApplyCamera(float DeltaSeconds)
{
	if (!OwnerCharacter)
	{
		OwnerCharacter = Cast<ACharacter>(GetOwner());
	}

	APlayerController* PC = OwnerCharacter ? Cast<APlayerController>(OwnerCharacter->GetController()) : nullptr;
	if (!PC || !CameraBoom || !FollowCamera) return;

	CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, DesiredArmLength, DeltaSeconds, ArmInterpSpeed);
	CameraBoom->SocketOffset = FMath::VInterpTo(CameraBoom->SocketOffset, DesiredSocketOffset, DeltaSeconds, OffsetInterpSpeed);

	float RotationInterpSpeed = RotationInterpSpeed_Explore;
	switch (CameraState)
	{
	case ECombatCameraState::Explore:
		RotationInterpSpeed = RotationInterpSpeed_Explore;
		break;
	case ECombatCameraState::CombatFree:
	case ECombatCameraState::Recover:
		RotationInterpSpeed = RotationInterpSpeed_Combat;
		break;
	case ECombatCameraState::LockOn:
		RotationInterpSpeed = GetRotationInterpSpeedForZone(CurrentZone);
		if (RotationInterpSpeed_Lock_Center <= 0.f)
		{
			if (!bLockCameraRotationActive)
			{
				RotationInterpSpeed = 0.f;
			}
			else if (CurrentZone == ELockOnZone::Center)
			{
				RotationInterpSpeed = RotationInterpSpeed_Lock_Buffer;
			}
		}
		break;
	default:
		break;
	}

	FRotator NewControlRot = PC->GetControlRotation();
	if (RotationInterpSpeed > 0.f)
	{
		NewControlRot = FMath::RInterpTo(NewControlRot, DesiredControlRotation, DeltaSeconds, RotationInterpSpeed);
	}

	if (CameraState == ECombatCameraState::Explore)
	{
		NewControlRot.Pitch = FMath::ClampAngle(NewControlRot.Pitch, ExploreMinPitch, ExploreMaxPitch);
	}
	else if (CameraState == ECombatCameraState::CombatFree || CameraState == ECombatCameraState::Recover)
	{
		NewControlRot.Pitch = FMath::ClampAngle(NewControlRot.Pitch, CombatMinPitch, CombatMaxPitch);
	}
	else
	{
		NewControlRot.Pitch = FMath::ClampAngle(NewControlRot.Pitch, LockMinPitch, LockMaxPitch);
	}

	PC->SetControlRotation(NewControlRot);

	const float NewFOV = FMath::FInterpTo(FollowCamera->FieldOfView, DesiredFOV, DeltaSeconds, FOVInterpSpeed);
	FollowCamera->SetFieldOfView(NewFOV);
}

bool URDTargetingCameraComponent::HasNearbyCombatTarget() const
{
	return GatherCandidateTargets(CombatDetectRadius).Num() > 0;
}

bool URDTargetingCameraComponent::IsValidLockTarget(AActor* Actor) const
{
	if (!OwnerCharacter || !Actor || Actor == OwnerCharacter || Actor->IsPendingKillPending() || !Actor->ActorHasTag(LockTargetTag))
	{
		return false;
	}

	const FVector PlayerPivot = GetPlayerPivot();
	const FVector TargetPivot = GetTargetPivot(Actor);
	const float Dist2D = FVector(TargetPivot.X - PlayerPivot.X, TargetPivot.Y - PlayerPivot.Y, 0.f).Length();
	if (Dist2D > LockAcquireRadius)
	{
		return false;
	}

	return FMath::Abs(TargetPivot.Z - PlayerPivot.Z) <= MaxLockVerticalDelta;
}

TArray<AActor*> URDTargetingCameraComponent::GatherCandidateTargets(float Radius) const
{
	TArray<AActor*> Results;
	if (!OwnerCharacter || !GetWorld()) return Results;

	TArray<FOverlapResult> Overlaps;
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerCharacter);

	const bool bHit = GetWorld()->OverlapMultiByObjectType(
		Overlaps,
		OwnerCharacter->GetActorLocation(),
		FQuat::Identity,
		ObjectQueryParams,
		FCollisionShape::MakeSphere(Radius),
		QueryParams);

	if (!bHit) return Results;

	for (const FOverlapResult& Overlap : Overlaps)
	{
		if (AActor* HitActor = Overlap.GetActor())
		{
			if (IsValidLockTarget(HitActor))
			{
				Results.AddUnique(HitActor);
			}
		}
	}

	return Results;
}

AActor* URDTargetingCameraComponent::FindBestLockTarget() const
{
	TArray<AActor*> Candidates = GatherCandidateTargets(LockAcquireRadius);
	if (Candidates.IsEmpty() || !OwnerCharacter) return nullptr;

	APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController());
	if (!PC) return nullptr;

	const FVector PlayerPivot = GetPlayerPivot();
	const FVector CamForward = FRotationMatrix(PC->GetControlRotation()).GetUnitAxis(EAxis::X);

	AActor* BestTarget = nullptr;
	float BestScore = -FLT_MAX;

	for (AActor* Candidate : Candidates)
	{
		const FVector TargetPivot = GetTargetPivot(Candidate);
		const FVector ToTarget = (TargetPivot - PlayerPivot).GetSafeNormal();
		const float Distance = FVector::Dist2D(PlayerPivot, TargetPivot);
		const float Dot = FVector::DotProduct(CamForward, ToTarget);
		float Score = (Dot * 1000.f) - Distance;

		if (Dot > 0.f)
		{
			Score += 200.f;
		}

		if (Score > BestScore)
		{
			BestScore = Score;
			BestTarget = Candidate;
		}
	}

	return BestTarget;
}

AActor* URDTargetingCameraComponent::FindSwitchTarget(float DirectionSign) const
{
	TArray<AActor*> Candidates = GatherCandidateTargets(LockAcquireRadius);
	if (Candidates.IsEmpty() || !CurrentLockTarget || !OwnerCharacter) return nullptr;

	APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController());
	if (!PC) return nullptr;

	const FVector PlayerPivot = GetPlayerPivot();
	const FVector CamForward = FRotationMatrix(PC->GetControlRotation()).GetUnitAxis(EAxis::X);
	const FVector CamRight = FRotationMatrix(PC->GetControlRotation()).GetUnitAxis(EAxis::Y);

	AActor* BestTarget = nullptr;
	float BestScore = -FLT_MAX;

	for (AActor* Candidate : Candidates)
	{
		if (Candidate == CurrentLockTarget)
		{
			continue;
		}

		const FVector ToTarget = (GetTargetPivot(Candidate) - PlayerPivot).GetSafeNormal();
		const float Side = FVector::DotProduct(CamRight, ToTarget);
		if ((DirectionSign > 0.f && Side <= 0.f) || (DirectionSign < 0.f && Side >= 0.f))
		{
			continue;
		}

		const float Front = FVector::DotProduct(CamForward, ToTarget);
		const float Distance = FVector::Dist2D(PlayerPivot, GetTargetPivot(Candidate));
		const float Score = Front * 1000.f - Distance + FMath::Abs(Side) * 100.f;

		if (Score > BestScore)
		{
			BestScore = Score;
			BestTarget = Candidate;
		}
	}

	return BestTarget ? BestTarget : CurrentLockTarget.Get();
}

ELockOnZone URDTargetingCameraComponent::ComputeLockZone(float TargetYawDeg) const
{
	if (!OwnerCharacter) return ELockOnZone::Center;

	APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController());
	if (!PC) return ELockOnZone::Center;

	const float CurrentYaw = PC->GetControlRotation().Yaw;
	const float DeltaYaw = FMath::Abs(FMath::FindDeltaAngleDegrees(CurrentYaw, TargetYawDeg));

	if (DeltaYaw <= CenterAngle) return ELockOnZone::Center;
	if (DeltaYaw <= BufferAngle) return ELockOnZone::Buffer;
	if (DeltaYaw <= EdgeAngle) return ELockOnZone::Edge;
	return ELockOnZone::Offscreen;
}

float URDTargetingCameraComponent::GetRotationInterpSpeedForZone(ELockOnZone Zone) const
{
	switch (Zone)
	{
	case ELockOnZone::Center:
		return RotationInterpSpeed_Lock_Center;
	case ELockOnZone::Buffer:
		return RotationInterpSpeed_Lock_Buffer;
	case ELockOnZone::Edge:
		return RotationInterpSpeed_Lock_Edge;
	case ELockOnZone::Offscreen:
		return RotationInterpSpeed_Lock_Offscreen;
	default:
		return RotationInterpSpeed_Lock_Center;
	}
}

void URDTargetingCameraComponent::DrawLockAngleDebug(float TargetYawDeg) const
{
	if (!bDebugDrawLockAngle || !OwnerCharacter || !CurrentLockTarget || !GetWorld())
	{
		return;
	}

	const APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController());
	if (!PC)
	{
		return;
	}

	const FVector PlayerPivot = GetPlayerPivot();
	const FVector TargetPivot = GetTargetPivot(CurrentLockTarget);
	const FVector DebugOrigin = FVector(PlayerPivot.X, PlayerPivot.Y, PlayerPivot.Z + 35.f);
	const FVector TargetPoint = FVector(TargetPivot.X, TargetPivot.Y, DebugOrigin.Z);

	const FRotator ControlYawRot(0.f, PC->GetControlRotation().Yaw, 0.f);
	const FVector ControlForward = FRotationMatrix(ControlYawRot).GetUnitAxis(EAxis::X).GetSafeNormal2D();
	const FVector ToTarget = (TargetPoint - DebugOrigin).GetSafeNormal2D();
	if (ControlForward.IsNearlyZero() || ToTarget.IsNearlyZero())
	{
		return;
	}

	const float DeltaYaw = FMath::Abs(FMath::FindDeltaAngleDegrees(PC->GetControlRotation().Yaw, TargetYawDeg));

	FColor ZoneColor = FColor::Green;
	switch (CurrentZone)
	{
	case ELockOnZone::Center:
		ZoneColor = FColor::Green;
		break;
	case ELockOnZone::Buffer:
		ZoneColor = FColor::Yellow;
		break;
	case ELockOnZone::Edge:
		ZoneColor = FColor::Orange;
		break;
	case ELockOnZone::Offscreen:
		ZoneColor = FColor::Red;
		break;
	default:
		break;
	}

	const FVector ControlEnd = DebugOrigin + ControlForward * DebugDrawLength;
	const FVector TargetDirectionEnd = DebugOrigin + ToTarget * DebugDrawLength;

	DrawDebugDirectionalArrow(GetWorld(), DebugOrigin, ControlEnd, 40.f, FColor::Cyan, false, 0.f, 0, 3.f);
	DrawDebugDirectionalArrow(GetWorld(), DebugOrigin, TargetDirectionEnd, 40.f, ZoneColor, false, 0.f, 0, 3.f);
	DrawDebugLine(GetWorld(), DebugOrigin, TargetPoint, ZoneColor, false, 0.f, 0, 1.5f);
	DrawDebugSphere(GetWorld(), DebugOrigin, 18.f, 12, FColor::Cyan, false, 0.f, 0, 1.5f);
	DrawDebugSphere(GetWorld(), TargetPoint, 18.f, 12, ZoneColor, false, 0.f, 0, 1.5f);
	DrawDebugString(
		GetWorld(),
		DebugOrigin + FVector(0.f, 0.f, 65.f),
		FString::Printf(TEXT("Lock DeltaYaw: %.1f | Center: %.1f | Buffer: %.1f | Edge: %.1f | RotActive: %s"), DeltaYaw, CenterAngle, BufferAngle, EdgeAngle, bLockCameraRotationActive ? TEXT("true") : TEXT("false")),
		nullptr,
		ZoneColor,
		0.f,
		true);
}

FVector URDTargetingCameraComponent::GetPlayerPivot() const
{
	return OwnerCharacter ? OwnerCharacter->GetActorLocation() + FVector(0.f, 0.f, 70.f) : FVector::ZeroVector;
}

FVector URDTargetingCameraComponent::GetTargetPivot(AActor* Target) const
{
	return Target ? Target->GetActorLocation() + FVector(0.f, 0.f, 70.f) : FVector::ZeroVector;
}

float URDTargetingCameraComponent::GetDistanceToTarget2D(AActor* Target) const
{
	if (!Target) return BIG_NUMBER;
	return FVector::Dist2D(GetPlayerPivot(), GetTargetPivot(Target));
}

bool URDTargetingCameraComponent::IsTargetInFrontHemisphere(AActor* Target) const
{
	if (!Target || !OwnerCharacter) return false;

	APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController());
	if (!PC) return false;

	const FVector CamForward = FRotationMatrix(PC->GetControlRotation()).GetUnitAxis(EAxis::X);
	const FVector ToTarget = (GetTargetPivot(Target) - GetPlayerPivot()).GetSafeNormal();
	return FVector::DotProduct(CamForward, ToTarget) > 0.f;
}

void URDTargetingCameraComponent::EnterLockOn(AActor* NewTarget)
{
	if (!NewTarget) return;

	CurrentLockTarget = NewTarget;
	PreviousLockTarget = nullptr;
	UnlockRecoveryRemaining = 0.f;
	LockOrbitYaw = 0.f;
	LockOrbitPitch = 0.f;
	PendingOrbitYawInput = 0.f;
	PendingOrbitPitchInput = 0.f;
	CurrentLockDeltaYaw = 0.f;
	bLockCameraRotationActive = false;
}

void URDTargetingCameraComponent::ExitLockOn()
{
	PreviousLockTarget = CurrentLockTarget;
	CurrentLockTarget = nullptr;
	LockOrbitYaw = 0.f;
	LockOrbitPitch = 0.f;
	PendingOrbitYawInput = 0.f;
	PendingOrbitPitchInput = 0.f;
	CurrentLockDeltaYaw = 0.f;
	bLockCameraRotationActive = false;
	UnlockRecoveryRemaining = UnlockRecoveryDelay;
}
