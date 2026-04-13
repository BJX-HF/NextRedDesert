#include "RDCharacterAnimationInstance.h"
#include "Animation/AnimMontage.h"
#include "AITestProject.h"

bool URDCharacterAnimationInstance::PlayActionMontage()
{
	if (!ActionMontage)
	{
		UE_LOG(LogAITestProject, Warning, TEXT("ActionMontage is null. Please assign montage in animation blueprint."));
		return false;
	}

	const float MontageLength = Montage_Play(ActionMontage, 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, true);
	if (MontageLength <= 0.0f)
	{
		UE_LOG(LogAITestProject, Warning, TEXT("Failed to play ActionMontage."));
		return false;
	}

	return true;
}
