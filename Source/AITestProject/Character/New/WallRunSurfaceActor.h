#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WallRunSurfaceActor.generated.h"

class USceneComponent;
class UWallRunSurfaceComponent;

UCLASS()
class AITESTPROJECT_API AWallRunSurfaceActor : public AActor
{
	GENERATED_BODY()

public:
	AWallRunSurfaceActor();

	UFUNCTION(BlueprintPure, Category = "WallRun")
	UWallRunSurfaceComponent* GetWallRunSurface() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WallRun")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WallRun")
	TObjectPtr<UWallRunSurfaceComponent> WallRunSurface;
};
