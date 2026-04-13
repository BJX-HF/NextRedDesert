#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Grab.generated.h"

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
};
