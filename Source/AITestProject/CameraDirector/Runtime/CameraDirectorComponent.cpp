#include "CameraDirector/Runtime/CameraDirectorComponent.h"

#include "Camera/CameraComponent.h"
#include "CameraDirector/Assets/CameraLogicAsset.h"
#include "CameraDirector/Assets/CameraStateMachineAsset.h"
#include "CameraDirector/Runtime/CameraLogicPlayer.h"
#include "CameraDirector/Runtime/CameraStateMachineRuntime.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/SpringArmComponent.h"

DEFINE_LOG_CATEGORY(LogCameraDirector);

UCameraDirectorComponent::UCameraDirectorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	LogicPlayer = CreateDefaultSubobject<UCameraLogicPlayer>(TEXT("CameraLogicPlayer"));
	StateMachineRuntime = new FCameraStateMachineRuntime();
}

UCameraDirectorComponent::~UCameraDirectorComponent()
{
	delete StateMachineRuntime;
	StateMachineRuntime = nullptr;
}

void UCameraDirectorComponent::BeginPlay()
{
	Super::BeginPlay();

	AutoResolveCameraTargets();

	if (LogicPlayer)
	{
		LogicPlayer->Initialize(this);
	}

	if (StateMachineRuntime)
	{
		StateMachineRuntime->Initialize(StateMachineAsset);
	}

	if (StateMachineAsset)
	{
		TArray<FString> ValidationErrors;
		if (!StateMachineAsset->ValidateAsset(ValidationErrors))
		{
			for (const FString& Error : ValidationErrors)
			{
				UE_LOG(LogCameraDirector, Warning, TEXT("CameraStateMachineAsset '%s': %s"), *GetNameSafe(StateMachineAsset), *Error);
			}
		}
	}

	if (!TargetSpringArm && !TargetCamera)
	{
		UE_LOG(LogCameraDirector, Warning, TEXT("CameraDirectorComponent on '%s' has no SpringArm or Camera assigned."), *GetNameSafe(GetOwner()));
	}

	CurrentCameraResult = BuildFallbackCameraResult();
}

void UCameraDirectorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!StateMachineRuntime)
	{
		return;
	}

	const FCameraStateContext Context = BuildContext(DeltaTime);
	StateMachineRuntime->Tick(Context);

	if (LogicPlayer)
	{
		LogicPlayer->Tick(Context);
	}

	const FCameraResult BaseResult = StateMachineAsset ? StateMachineRuntime->GetCurrentResult() : BuildFallbackCameraResult();
	CurrentCameraResult = LogicPlayer ? LogicPlayer->ApplyCameraLogic(Context, BaseResult) : BaseResult;
	ApplyCameraResult(CurrentCameraResult);

	if (TargetCamera)
	{
		CurrentCameraResult.CameraLocation = TargetCamera->GetComponentLocation();
		CurrentCameraResult.CameraRotation = TargetCamera->GetComponentRotation();
	}
}

bool UCameraDirectorComponent::PlayCameraLogic(UCameraLogicAsset* Logic)
{
	return LogicPlayer ? LogicPlayer->PlayCameraLogic(Logic, BuildContext(0.0f)) : false;
}

void UCameraDirectorComponent::StopCameraLogicBySlot(FName SlotName)
{
	if (LogicPlayer)
	{
		LogicPlayer->StopCameraLogicBySlot(SlotName);
	}
}

void UCameraDirectorComponent::StopAllCameraLogic()
{
	if (LogicPlayer)
	{
		LogicPlayer->StopAllCameraLogic();
	}
}

void UCameraDirectorComponent::ForceSetBaseState(FName StateName)
{
	if (StateMachineRuntime)
	{
		StateMachineRuntime->ForceSetState(StateName);
	}
}

FName UCameraDirectorComponent::GetCurrentBaseState() const
{
	return StateMachineRuntime ? StateMachineRuntime->GetCurrentStateName() : NAME_None;
}

FCameraResult UCameraDirectorComponent::GetCurrentCameraResult() const
{
	return CurrentCameraResult;
}

void UCameraDirectorComponent::SetLockTarget(AActor* NewLockTarget)
{
	LockTargetActor = NewLockTarget;
}

void UCameraDirectorComponent::SetCameraFlags(bool bInIsSprinting, bool bInIsAttacking, bool bInIsHitReact, bool bInWantsLockOn)
{
	bIsSprinting = bInIsSprinting;
	bIsAttacking = bInIsAttacking;
	bIsHitReact = bInIsHitReact;
	bWantsLockOn = bInWantsLockOn;
}

FCameraStateContext UCameraDirectorComponent::BuildContext(float DeltaTime) const
{
	FCameraStateContext Context;
	Context.OwnerActor = GetOwner();
	Context.OwnerCharacter = Cast<ACharacter>(GetOwner());
	Context.DeltaTime = DeltaTime;

	if (const AActor* OwnerActor = GetOwner())
	{
		Context.ActorLocation = OwnerActor->GetActorLocation();
		Context.Velocity = OwnerActor->GetVelocity();
		Context.Speed = Context.Velocity.Size2D();
		Context.ControlRotation = OwnerActor->GetActorRotation();
	}

	if (const APawn* PawnOwner = Cast<APawn>(GetOwner()))
	{
		if (const AController* Controller = PawnOwner->GetController())
		{
			Context.ControlRotation = Controller->GetControlRotation();
		}
	}

	if (const ACharacter* Character = Context.OwnerCharacter)
	{
		if (const UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement())
		{
			Context.Acceleration = MovementComponent->GetCurrentAcceleration();
			Context.bIsInAir = MovementComponent->IsFalling();
			Context.bIsAccelerating = !Context.Acceleration.IsNearlyZero();
		}
	}

	Context.LockTargetActor = LockTargetActor.Get();
	Context.bHasLockTarget = Context.LockTargetActor != nullptr;
	Context.LockTargetLocation = Context.bHasLockTarget ? Context.LockTargetActor->GetActorLocation() : FVector::ZeroVector;
	Context.bIsSprinting = bIsSprinting;
	Context.bIsAttacking = bIsAttacking;
	Context.bIsHitReact = bIsHitReact;
	Context.bWantsLockOn = bWantsLockOn;
	return Context;
}

FCameraResult UCameraDirectorComponent::BuildFallbackCameraResult() const
{
	FCameraResult Result;
	Result.CameraLocation = TargetCamera ? TargetCamera->GetComponentLocation() : (GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector);
	Result.CameraRotation = TargetCamera ? TargetCamera->GetComponentRotation() : (GetOwner() ? GetOwner()->GetActorRotation() : FRotator::ZeroRotator);
	Result.FOV = TargetCamera ? TargetCamera->FieldOfView : 70.0f;

	if (TargetSpringArm)
	{
		Result.TargetArmLength = TargetSpringArm->TargetArmLength;
		Result.SocketOffset = TargetSpringArm->SocketOffset;
		Result.TargetOffset = TargetSpringArm->TargetOffset;
		Result.CameraLagSpeed = TargetSpringArm->CameraLagSpeed;
		Result.CameraRotationLagSpeed = TargetSpringArm->CameraRotationLagSpeed;
		Result.bUsePawnControlRotation = TargetSpringArm->bUsePawnControlRotation;
	}

	return Result;
}

void UCameraDirectorComponent::ApplyCameraResult(const FCameraResult& Result)
{
	if (TargetSpringArm)
	{
		TargetSpringArm->TargetArmLength = Result.TargetArmLength;
		TargetSpringArm->SocketOffset = Result.SocketOffset;
		TargetSpringArm->TargetOffset = Result.TargetOffset;
		TargetSpringArm->CameraLagSpeed = Result.CameraLagSpeed;
		TargetSpringArm->CameraRotationLagSpeed = Result.CameraRotationLagSpeed;
		TargetSpringArm->bUsePawnControlRotation = Result.bUsePawnControlRotation;
		TargetSpringArm->bEnableCameraLag = Result.CameraLagSpeed > 0.0f;
		TargetSpringArm->bEnableCameraRotationLag = Result.CameraRotationLagSpeed > 0.0f;
	}

	if (TargetCamera)
	{
		TargetCamera->SetFieldOfView(Result.FOV);

		if (!TargetSpringArm)
		{
			TargetCamera->SetWorldLocationAndRotation(Result.CameraLocation, Result.CameraRotation);
		}
	}

	if (APawn* PawnOwner = Cast<APawn>(GetOwner()))
	{
		if (AController* Controller = PawnOwner->GetController())
		{
			Controller->SetControlRotation(Result.CameraRotation);
			return;
		}
	}

	if (TargetSpringArm && !TargetSpringArm->bUsePawnControlRotation)
	{
		TargetSpringArm->SetWorldRotation(Result.CameraRotation);
	}
}

void UCameraDirectorComponent::AutoResolveCameraTargets()
{
	if (!GetOwner())
	{
		return;
	}

	if (!TargetSpringArm)
	{
		TargetSpringArm = GetOwner()->FindComponentByClass<USpringArmComponent>();
	}

	if (!TargetCamera)
	{
		TargetCamera = GetOwner()->FindComponentByClass<UCameraComponent>();
	}
}
