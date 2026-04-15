#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Grab.generated.h"

class ARDBaseAICharacter;
class ACharacter;

UCLASS(Blueprintable)
class AITESTPROJECT_API UGA_Grab : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Grab();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

	UFUNCTION(BlueprintCallable, Category = "Grab")
	void ExecuteThrow();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grab", meta = (ClampMin = 0.0))
	float HoldDuration = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grab", meta = (ClampMin = 0.0))
	float ThrowForwardStrength = 900.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grab", meta = (ClampMin = 0.0))
	float ThrowUpStrength = 260.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grab")
	FName GrabAttachSocketName = TEXT("GrabSocket");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grab")
	FVector GrabAttachOffset = FVector(60.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grab")
	bool bDisableTargetMovementWhileHeld = true;

private:
	void ClearThrowTimer();
	void ReleaseTarget(bool bApplyThrow);

	FTimerHandle ThrowTimerHandle;
	TWeakObjectPtr<ACharacter> CachedOwnerCharacter;
	TWeakObjectPtr<ARDBaseAICharacter> GrabbedTarget;
	bool bHasExecutedThrow = false;
};
