#include "CameraDirector/Runtime/CameraLogicPlayer.h"

#include "CameraDirector/Assets/CameraLogicAsset.h"

void UCameraLogicPlayer::Initialize(UObject* InOwner)
{
	OwnerObject = InOwner;
	ActiveLogics.Reset();
	NextInstanceId = 1;
}

bool UCameraLogicPlayer::PlayCameraLogic(UCameraLogicAsset* LogicAsset, const FCameraStateContext& Context)
{
	if (!LogicAsset || !CanPlayLogic(LogicAsset))
	{
		return false;
	}

	if (LogicAsset->ApplyMode == ECameraLogicApplyMode::Override)
	{
		if (FActiveCameraLogic* CurrentOverride = FindOverrideLogic())
		{
			RequestStop(*CurrentOverride);
		}
	}
	else
	{
		for (FActiveCameraLogic& Entry : ActiveLogics)
		{
			if (Entry.LogicInstance && Entry.LogicInstance->ApplyMode == ECameraLogicApplyMode::Additive && Entry.LogicInstance->SlotName == LogicAsset->SlotName)
			{
				RequestStop(Entry);
			}
		}
	}

	UCameraLogicAsset* LogicInstance = DuplicateObject<UCameraLogicAsset>(LogicAsset, this);
	if (!LogicInstance)
	{
		return false;
	}

	LogicInstance->ResetRuntimeState();
	LogicInstance->OnActivated(Context);

	FActiveCameraLogic& NewEntry = ActiveLogics.AddDefaulted_GetRef();
	NewEntry.LogicInstance = LogicInstance;
	NewEntry.InstanceId = NextInstanceId++;
	NewEntry.BlendElapsed = 0.0f;
	NewEntry.BlendAlpha = 0.0f;
	NewEntry.bIsStopping = false;
	return true;
}

void UCameraLogicPlayer::StopCameraLogicBySlot(FName SlotName)
{
	for (FActiveCameraLogic& Entry : ActiveLogics)
	{
		if (Entry.LogicInstance && Entry.LogicInstance->SlotName == SlotName)
		{
			RequestStop(Entry);
		}
	}
}

void UCameraLogicPlayer::StopAllCameraLogic()
{
	for (FActiveCameraLogic& Entry : ActiveLogics)
	{
		RequestStop(Entry);
	}
}

void UCameraLogicPlayer::Tick(const FCameraStateContext& Context)
{
	for (FActiveCameraLogic& Entry : ActiveLogics)
	{
		if (!Entry.LogicInstance)
		{
			continue;
		}

		Entry.LogicInstance->AdvanceRuntime(Context.DeltaTime);
		Entry.LogicInstance->TickLogic(Context);

		if (!Entry.bIsStopping && Entry.LogicInstance->IsFinished(Context))
		{
			RequestStop(Entry);
		}

		const float BlendDuration = Entry.bIsStopping ? Entry.LogicInstance->BlendOut.BlendTime : Entry.LogicInstance->BlendIn.BlendTime;
		if (BlendDuration <= KINDA_SMALL_NUMBER)
		{
			Entry.BlendAlpha = Entry.bIsStopping ? 0.0f : 1.0f;
		}
		else
		{
			Entry.BlendElapsed += Context.DeltaTime;
			const float LinearAlpha = FMath::Clamp(Entry.BlendElapsed / BlendDuration, 0.0f, 1.0f);
			Entry.BlendAlpha = Entry.bIsStopping
				? 1.0f - Entry.LogicInstance->BlendOut.EvaluateAlpha(LinearAlpha)
				: Entry.LogicInstance->BlendIn.EvaluateAlpha(LinearAlpha);
		}
	}

	CleanupFinished(Context);
}

FCameraResult UCameraLogicPlayer::ApplyCameraLogic(const FCameraStateContext& Context, const FCameraResult& BaseCamera) const
{
	FCameraResult Result = BaseCamera;

	if (const FActiveCameraLogic* OverrideEntry = FindOverrideLogic())
	{
		if (OverrideEntry->LogicInstance && OverrideEntry->BlendAlpha > 0.0f)
		{
			FCameraResult OverrideResult = BaseCamera;
			OverrideEntry->LogicInstance->EvaluateCamera(Context, BaseCamera, OverrideResult);
			Result = FCameraResult::Blend(BaseCamera, OverrideResult, OverrideEntry->BlendAlpha);
		}
	}

	TArray<const FActiveCameraLogic*> Additives;
	for (const FActiveCameraLogic& Entry : ActiveLogics)
	{
		if (Entry.LogicInstance && Entry.LogicInstance->ApplyMode == ECameraLogicApplyMode::Additive && Entry.BlendAlpha > 0.0f)
		{
			Additives.Add(&Entry);
		}
	}

	Additives.Sort([](const FActiveCameraLogic& A, const FActiveCameraLogic& B)
	{
		if (!A.LogicInstance || !B.LogicInstance)
		{
			return A.LogicInstance != nullptr;
		}

		if (A.LogicInstance->Priority != B.LogicInstance->Priority)
		{
			return A.LogicInstance->Priority > B.LogicInstance->Priority;
		}

		return A.InstanceId < B.InstanceId;
	});

	for (const FActiveCameraLogic* Entry : Additives)
	{
		FCameraResult Modified = Result;
		Entry->LogicInstance->EvaluateCamera(Context, BaseCamera, Modified);
		Result = FCameraResult::Blend(Result, Modified, Entry->BlendAlpha);
	}

	return Result;
}

bool UCameraLogicPlayer::HasActiveLogic(FName SlotName) const
{
	return ActiveLogics.ContainsByPredicate([SlotName](const FActiveCameraLogic& Entry)
	{
		return Entry.LogicInstance && Entry.LogicInstance->SlotName == SlotName;
	});
}

bool UCameraLogicPlayer::CanPlayLogic(const UCameraLogicAsset* LogicAsset) const
{
	if (!LogicAsset)
	{
		return false;
	}

	if (LogicAsset->ApplyMode == ECameraLogicApplyMode::Override)
	{
		if (const FActiveCameraLogic* CurrentOverride = FindOverrideLogic())
		{
			if (CurrentOverride->LogicInstance && CurrentOverride->LogicInstance->Priority > LogicAsset->Priority)
			{
				return CurrentOverride->LogicInstance->CanBeInterruptedBy(LogicAsset);
			}
		}
	}

	return true;
}

void UCameraLogicPlayer::RequestStop(FActiveCameraLogic& Entry)
{
	if (!Entry.LogicInstance || Entry.bIsStopping)
	{
		return;
	}

	Entry.bIsStopping = true;
	Entry.BlendElapsed = 0.0f;
	Entry.LogicInstance->SetRuntimeStopping(true);
}

void UCameraLogicPlayer::CleanupFinished(const FCameraStateContext& Context)
{
	for (int32 Index = ActiveLogics.Num() - 1; Index >= 0; --Index)
	{
		FActiveCameraLogic& Entry = ActiveLogics[Index];
		if (!Entry.LogicInstance)
		{
			ActiveLogics.RemoveAt(Index);
			continue;
		}

		if (Entry.bIsStopping && Entry.BlendAlpha <= KINDA_SMALL_NUMBER)
		{
			Entry.LogicInstance->OnDeactivated(Context);
			ActiveLogics.RemoveAt(Index);
		}
	}
}

FActiveCameraLogic* UCameraLogicPlayer::FindOverrideLogic()
{
	return ActiveLogics.FindByPredicate([](const FActiveCameraLogic& Entry)
	{
		return Entry.LogicInstance && Entry.LogicInstance->ApplyMode == ECameraLogicApplyMode::Override;
	});
}

const FActiveCameraLogic* UCameraLogicPlayer::FindOverrideLogic() const
{
	return ActiveLogics.FindByPredicate([](const FActiveCameraLogic& Entry)
	{
		return Entry.LogicInstance && Entry.LogicInstance->ApplyMode == ECameraLogicApplyMode::Override;
	});
}
