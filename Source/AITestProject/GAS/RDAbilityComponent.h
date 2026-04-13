#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "RDAbilityComponent.generated.h"

class AActor;
class UGameplayAbility;

UCLASS(ClassGroup=(Abilities), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class AITESTPROJECT_API URDAbilityComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	URDAbilityComponent();

	UFUNCTION(BlueprintCallable, Category="Ability")
	bool TryStartDash();

	UFUNCTION(BlueprintCallable, Category="Grab")
	bool TryStartGrab();

	UFUNCTION(BlueprintPure, Category="Grab")
	bool CanStartGrab(AActor* Target) const;

	UFUNCTION(BlueprintCallable, Category="Grab")
	AActor* FindBestGrabTarget() const;

	UFUNCTION(BlueprintPure, Category="Grab")
	AActor* GetCurrentGrabTarget() const { return CurrentGrabTarget.Get(); }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ability|Dash")
	TSubclassOf<UGameplayAbility> DashAbilityClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ability|Grab")
	TSubclassOf<UGameplayAbility> GrabAbilityClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Grab|Condition", meta=(ClampMin="0.0", Units="cm"))
	float GrabRange = 220.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Grab|Condition", meta=(ClampMin="0.0", ClampMax="180.0", Units="deg"))
	float GrabHalfAngleDeg = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Grab|Condition", meta=(ClampMin="0.0", Units="cm"))
	float MaxVerticalDelta = 150.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Grab")
	TWeakObjectPtr<AActor> CurrentGrabTarget;
};
