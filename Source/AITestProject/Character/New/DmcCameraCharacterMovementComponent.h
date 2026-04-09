#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DmcCameraCharacterMovementComponent.generated.h"

UCLASS()
class AITESTPROJECT_API UDmcCameraCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	virtual void PhysCustom(float DeltaTime, int32 Iterations) override;

protected:
	void PhysWallRun(float DeltaTime, int32 Iterations);
};
