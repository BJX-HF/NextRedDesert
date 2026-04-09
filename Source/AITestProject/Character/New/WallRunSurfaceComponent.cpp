#include "Character/New/WallRunSurfaceComponent.h"

#include "DrawDebugHelpers.h"

UWallRunSurfaceComponent::UWallRunSurfaceComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UWallRunSurfaceComponent::OnRegister()
{
	Super::OnRegister();
	ApplyDefaultCollisionSettings();
}

void UWallRunSurfaceComponent::OnComponentCreated()
{
	Super::OnComponentCreated();
	ApplyDefaultCollisionSettings();
}

void UWallRunSurfaceComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bDebugDraw)
	{
		DrawDebugVisualization();
	}
}

bool UWallRunSurfaceComponent::IsWallRunEnabled() const
{
	return bEnabled;
}

FVector UWallRunSurfaceComponent::GetWallNormal() const
{
	switch (NormalAxis)
	{
	case EWallRunNormalAxis::Forward:
		return GetForwardVector().GetSafeNormal();
	case EWallRunNormalAxis::Backward:
		return -GetForwardVector().GetSafeNormal();
	case EWallRunNormalAxis::Left:
		return -GetRightVector().GetSafeNormal();
	case EWallRunNormalAxis::Right:
	default:
		return GetRightVector().GetSafeNormal();
	}
}

FVector UWallRunSurfaceComponent::GetPositiveWallRunDirection() const
{
	return FVector::CrossProduct(FVector::UpVector, GetWallNormal()).GetSafeNormal();
}

FVector UWallRunSurfaceComponent::GetWallRunDirectionFromVelocity(const FVector& Velocity) const
{
	const FVector PositiveDirection = GetPositiveWallRunDirection();
	if (PositiveDirection.IsNearlyZero())
	{
		return FVector::ZeroVector;
	}

	switch (AllowedDirection)
	{
	case EWallRunAllowedDirection::PositiveOnly:
		return PositiveDirection;
	case EWallRunAllowedDirection::NegativeOnly:
		return -PositiveDirection;
	case EWallRunAllowedDirection::Bidirectional:
	default:
	{
		FVector ReferenceVelocity = FVector(Velocity.X, Velocity.Y, 0.f).GetSafeNormal();
		if (ReferenceVelocity.IsNearlyZero())
		{
			return PositiveDirection;
		}

		return FVector::DotProduct(PositiveDirection, ReferenceVelocity) >= 0.f
			? PositiveDirection
			: -PositiveDirection;
	}
	}
}

bool UWallRunSurfaceComponent::IsDirectionAllowed(const FVector& WallRunDirection) const
{
	if (AllowedDirection == EWallRunAllowedDirection::Bidirectional)
	{
		return true;
	}

	const FVector PositiveDirection = GetPositiveWallRunDirection();
	if (PositiveDirection.IsNearlyZero() || WallRunDirection.IsNearlyZero())
	{
		return false;
	}

	const float Dot = FVector::DotProduct(PositiveDirection, WallRunDirection.GetSafeNormal());
	return AllowedDirection == EWallRunAllowedDirection::PositiveOnly ? Dot >= 0.85f : Dot <= -0.85f;
}

bool UWallRunSurfaceComponent::IsPointOnLeftSide(const FVector& CharacterLocation) const
{
	const FVector ToCharacter = CharacterLocation - GetComponentLocation();
	return FVector::DotProduct(GetRightVector(), ToCharacter) < 0.f;
}

FVector UWallRunSurfaceComponent::GetClosestPointToLocation(const FVector& WorldLocation) const
{
	const FVector LocalPoint = GetComponentTransform().InverseTransformPosition(WorldLocation);
	const FVector Extent = GetUnscaledBoxExtent();
	const FVector ClampedPoint(
		FMath::Clamp(LocalPoint.X, -Extent.X, Extent.X),
		FMath::Clamp(LocalPoint.Y, -Extent.Y, Extent.Y),
		FMath::Clamp(LocalPoint.Z, -Extent.Z, Extent.Z));

	return GetComponentTransform().TransformPosition(ClampedPoint);
}

float UWallRunSurfaceComponent::ResolveWallRunSpeed(float DefaultSpeed) const
{
	return WallRunSpeedOverride > 0.f ? WallRunSpeedOverride : DefaultSpeed;
}

float UWallRunSurfaceComponent::ResolveWallRunMaxDuration(float DefaultDuration) const
{
	return WallRunMaxDurationOverride > 0.f ? WallRunMaxDurationOverride : DefaultDuration;
}

void UWallRunSurfaceComponent::ApplyDefaultCollisionSettings()
{
	SetBoxExtent(FVector(80.f, 200.f, 180.f));
	SetCanEverAffectNavigation(false);
	SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SetCollisionObjectType(ECC_WorldDynamic);
	SetCollisionResponseToAllChannels(ECR_Ignore);
	SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SetGenerateOverlapEvents(true);
}

void UWallRunSurfaceComponent::DrawDebugVisualization() const
{
	if (!GetWorld())
	{
		return;
	}

	const FVector Origin = GetComponentLocation();
	const FVector Normal = GetWallNormal();
	const FVector Tangent = GetPositiveWallRunDirection();

	DrawDebugDirectionalArrow(GetWorld(), Origin, Origin + Normal * 120.f, 20.f, FColor::Blue, false, -1.f, 0, 3.f);
	DrawDebugDirectionalArrow(GetWorld(), Origin, Origin + Tangent * 150.f, 20.f, FColor::Green, false, -1.f, 0, 3.f);
}
