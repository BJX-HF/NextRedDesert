#pragma once

#include "CoreMinimal.h"
#include "RDAbilityStateMachine.generated.h"

UENUM(BlueprintType)
enum class ERDAbilityMainState : uint8
{
	Normal,
	Airborne,
	HardCC,
	Dead
};

UENUM(BlueprintType)
enum class ERDSkillFlowState : uint8
{
	None,
	DashStartup,
	DashActive,
	DashRecovery,
	GrabAim,
	GrabLatch,
	GrabHold,
	GrabThrow,
	SkillRecovery
};

class AITESTPROJECT_API FRDAbilityStateMachine
{
public:
	ERDAbilityMainState GetMainState() const { return MainState; }
	ERDSkillFlowState GetSkillState() const { return SkillState; }

	void SetMainState(ERDAbilityMainState NewState);
	bool SetSkillState(ERDSkillFlowState NewState);
	bool CanEnterDash() const;
	bool CanEnterGrab() const;
	bool CanTransitSkillState(ERDSkillFlowState FromState, ERDSkillFlowState ToState) const;

private:
	ERDAbilityMainState MainState = ERDAbilityMainState::Normal;
	ERDSkillFlowState SkillState = ERDSkillFlowState::None;
};
