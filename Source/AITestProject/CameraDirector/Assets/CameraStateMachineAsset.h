#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CameraDirector/Types/CameraDirectorTypes.h"
#include "CameraStateMachineAsset.generated.h"

USTRUCT(BlueprintType)
struct AITESTPROJECT_API FCameraStateDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "State")
	FName StateName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "State")
	FCameraBasicParams Params;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "State")
	int32 Priority = 0;
};

USTRUCT(BlueprintType)
struct AITESTPROJECT_API FCameraTransitionDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transition")
	FName FromState = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transition")
	FName ToState = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transition")
	FCameraStateTransitionRule Rule;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transition")
	FCameraBlendSettings BlendSettings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transition")
	bool bCanInterrupt = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transition")
	int32 Priority = 0;
};

UCLASS(BlueprintType)
class AITESTPROJECT_API UCameraStateMachineAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "State Machine")
	FName EntryState = TEXT("Idle");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "State Machine")
	TArray<FCameraStateDefinition> States;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "State Machine")
	TArray<FCameraTransitionDefinition> Transitions;

	const FCameraStateDefinition* FindStateByName(FName StateName) const;
	TArray<const FCameraTransitionDefinition*> GetOutgoingTransitions(FName StateName) const;
	bool ValidateAsset(TArray<FString>& OutErrors) const;
};
