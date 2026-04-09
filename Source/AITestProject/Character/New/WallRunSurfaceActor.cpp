#include "Character/New/WallRunSurfaceActor.h"

#include "Character/New/WallRunSurfaceComponent.h"
#include "Components/SceneComponent.h"

AWallRunSurfaceActor::AWallRunSurfaceActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	WallRunSurface = CreateDefaultSubobject<UWallRunSurfaceComponent>(TEXT("WallRunSurface"));
	WallRunSurface->SetupAttachment(Root);
}

UWallRunSurfaceComponent* AWallRunSurfaceActor::GetWallRunSurface() const
{
	return WallRunSurface;
}
