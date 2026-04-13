#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "RDCharacterAnimationInstance.generated.h"

class UAnimMontage;

UCLASS(Blueprintable)
class URDCharacterAnimationInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Animation")
	bool PlayActionMontage();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Animation")
	UAnimMontage* ActionMontage;
};
