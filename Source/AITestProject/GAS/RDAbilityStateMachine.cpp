#include "GAS/RDAbilityStateMachine.h"

void FRDAbilityStateMachine::SetMainState(ERDAbilityMainState NewState)
{
	MainState = NewState;

	if (MainState == ERDAbilityMainState::HardCC || MainState == ERDAbilityMainState::Dead)
	{
		SkillState = ERDSkillFlowState::SkillRecovery;
	}
}

bool FRDAbilityStateMachine::CanTransitSkillState(ERDSkillFlowState FromState, ERDSkillFlowState ToState) const
{
	if (FromState == ToState)
	{
		return true;
	}

	if (ToState == ERDSkillFlowState::SkillRecovery)
	{
		return true;
	}

	switch (FromState)
	{
	case ERDSkillFlowState::None:
		return ToState == ERDSkillFlowState::DashStartup || ToState == ERDSkillFlowState::GrabAim;
	case ERDSkillFlowState::DashStartup:
		return ToState == ERDSkillFlowState::DashActive;
	case ERDSkillFlowState::DashActive:
		return ToState == ERDSkillFlowState::DashRecovery || ToState == ERDSkillFlowState::GrabAim;
	case ERDSkillFlowState::DashRecovery:
		return ToState == ERDSkillFlowState::None || ToState == ERDSkillFlowState::GrabAim;
	case ERDSkillFlowState::GrabAim:
		return ToState == ERDSkillFlowState::GrabLatch;
	case ERDSkillFlowState::GrabLatch:
		return ToState == ERDSkillFlowState::GrabHold;
	case ERDSkillFlowState::GrabHold:
		return ToState == ERDSkillFlowState::GrabThrow;
	case ERDSkillFlowState::GrabThrow:
		return ToState == ERDSkillFlowState::SkillRecovery;
	case ERDSkillFlowState::SkillRecovery:
		return ToState == ERDSkillFlowState::None || ToState == ERDSkillFlowState::DashStartup || ToState == ERDSkillFlowState::GrabAim;
	default:
		return false;
	}
}

bool FRDAbilityStateMachine::SetSkillState(ERDSkillFlowState NewState)
{
	if (!CanTransitSkillState(SkillState, NewState))
	{
		return false;
	}

	SkillState = NewState;
	return true;
}

bool FRDAbilityStateMachine::CanEnterDash() const
{
	if (MainState == ERDAbilityMainState::Dead || MainState == ERDAbilityMainState::HardCC)
	{
		return false;
	}

	return SkillState == ERDSkillFlowState::None
		|| SkillState == ERDSkillFlowState::DashRecovery
		|| SkillState == ERDSkillFlowState::SkillRecovery;
}

bool FRDAbilityStateMachine::CanEnterGrab() const
{
	if (MainState == ERDAbilityMainState::Dead || MainState == ERDAbilityMainState::HardCC)
	{
		return false;
	}

	return SkillState == ERDSkillFlowState::None
		|| SkillState == ERDSkillFlowState::DashActive
		|| SkillState == ERDSkillFlowState::DashRecovery
		|| SkillState == ERDSkillFlowState::SkillRecovery;
}
