#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "RDBaseAICharacter.generated.h"

UCLASS(Blueprintable)
class AITESTPROJECT_API ARDBaseAICharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ARDBaseAICharacter();

	UFUNCTION(BlueprintPure, Category = "Grab")
	bool CanBeGrabbed() const { return bCanBeGrabbed; }

	UFUNCTION(BlueprintCallable, Category = "Grab")
	void SetCanBeGrabbed(const bool bInCanBeGrabbed) { bCanBeGrabbed = bInCanBeGrabbed; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab")
	bool bCanBeGrabbed = false;
};


