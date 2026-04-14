#include "RDBasecharacter.h"

#include "GAS/RDAbilityComponent.h"
#include "Camera/CameraComponent.h"
#include "Character/New/RDBasecharacterMovementComponent.h"
#include "Character/New/RDTargetingCameraComponent.h"
#include "Character/New/WallRunSurfaceComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/PrimitiveComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/Engine.h"
#include "GameplayAbilitySpec.h"
#include "GameplayEffectTypes.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "GAS/AITestAttributeSet.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"

//测试修改git

ARDBasecharacter::ARDBasecharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<URDBasecharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f);
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 420.f;
	CameraBoom->SocketOffset = FVector(0.f, 50.f, 65.f);
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bDoCollisionTest = true;
	CameraBoom->ProbeSize = 12.f;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 12.f;
	CameraBoom->bEnableCameraRotationLag = false;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
	FollowCamera->FieldOfView = 60.f;

	TargetingCameraComponent = CreateDefaultSubobject<URDTargetingCameraComponent>(TEXT("TargetingCameraComponent"));
	TargetingCameraComponent->InitializeCamera(CameraBoom, FollowCamera);

	WallRunDetector = CreateDefaultSubobject<UBoxComponent>(TEXT("WallRunDetector"));
	WallRunDetector->SetupAttachment(GetCapsuleComponent());
	WallRunDetector->SetBoxExtent(FVector(65.f, 75.f, 95.f));
	WallRunDetector->SetRelativeLocation(FVector(0.f, 0.f, 10.f));
	WallRunDetector->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	WallRunDetector->SetCollisionObjectType(ECC_Pawn);
	WallRunDetector->SetCollisionResponseToAllChannels(ECR_Ignore);
	WallRunDetector->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	WallRunDetector->SetGenerateOverlapEvents(true);
	WallRunDetector->SetCanEverAffectNavigation(false);

	AbilitySystemComponent = CreateDefaultSubobject<URDAbilityComponent>(TEXT("RDAbilityComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UAITestAttributeSet>(TEXT("AttributeSet"));

	DefaultCapsuleHalfHeight = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	DefaultGroundFriction = GetCharacterMovement()->GroundFriction;
	DefaultBrakingDecelerationWalking = GetCharacterMovement()->BrakingDecelerationWalking;
	DefaultMaxWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;
}

UAbilitySystemComponent* ARDBasecharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}



void ARDBasecharacter::Jump()
{
	LastJumpPressedTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;

	if (CanPerformWallJump())
	{
		bWallJumpRequested = true;
		PerformWallJump();
		return;
	}

	Super::Jump();
}

void ARDBasecharacter::BeginPlay()
{
	Super::BeginPlay();

	InitializeAbilitySystem();

	DefaultCapsuleHalfHeight = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	DefaultGroundFriction = GetCharacterMovement()->GroundFriction;
	DefaultBrakingDecelerationWalking = GetCharacterMovement()->BrakingDecelerationWalking;
	DefaultMaxWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;
	GetCharacterMovement()->SetCrouchedHalfHeight(GetClampedSlideCapsuleHalfHeight());

	if (WallRunDetector)
	{
		WallRunDetector->OnComponentBeginOverlap.AddDynamic(this, &ARDBasecharacter::OnWallRunDetectorBeginOverlap);
		WallRunDetector->OnComponentEndOverlap.AddDynamic(this, &ARDBasecharacter::OnWallRunDetectorEndOverlap);
	}

	if (TargetingCameraComponent)
	{
		TargetingCameraComponent->InitializeCamera(CameraBoom, FollowCamera);
	}

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
				LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				if (DefaultMappingContext)
				{
					Subsystem->AddMappingContext(DefaultMappingContext, 0);
				}
			}
		}

	}
}

void ARDBasecharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	InitializeAbilitySystem();
}

void ARDBasecharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	HorizontalSpeed = FVector(GetVelocity().X, GetVelocity().Y, 0.f).Size();

	if (SlideCooldownRemaining > 0.f)
	{
		SlideCooldownRemaining -= DeltaSeconds;
	}

	UpdateSlide(DeltaSeconds);
	UpdateWallRunState(DeltaSeconds);
	UpdateCameraSystem(DeltaSeconds);
}

void ARDBasecharacter::Landed(const FHitResult& Hit)
{
	if (bIsWallRunning)
	{
		StopWallRun(false);
	}

	bWallJumpRequested = false;
	LastWallJumpSide = EWallRunSide::None;
	SetWallJumpTriggered(false);
	Super::Landed(Hit);
}

void ARDBasecharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	if (IsWallRunMovementMode())
	{
		bIsWallRunning = true;
		WallRunLastWallContactTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
		GetCharacterMovement()->bOrientRotationToMovement = false;
		return;
	}

	if (PrevMovementMode == MOVE_Custom && PreviousCustomMode == static_cast<uint8>(ERDBasecharacterCustomMovementMode::CMOVE_WallRun))
	{
		ClearWallRunState();
	}
}

void ARDBasecharacter::InitializeAbilitySystem()
{
	if (!AbilitySystemComponent || !AttributeSet)
	{
		return;
	}

	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UAITestAttributeSet::GetHealthAttribute()).RemoveAll(this);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UAITestAttributeSet::GetHealthAttribute()).AddUObject(this, &ARDBasecharacter::HandleHealthChanged);
	AbilitySystemComponent->SetNumericAttributeBase(UAITestAttributeSet::GetMaxHealthAttribute(), MaxHealth);
	AbilitySystemComponent->SetNumericAttributeBase(UAITestAttributeSet::GetHealthAttribute(), MaxHealth);

	CurrentHealth = AttributeSet->GetHealth();
	GrantStartupAbilities();
}

void ARDBasecharacter::GrantStartupAbilities()
{
	if (!AbilitySystemComponent || bStartupAbilitiesGranted || !HasAuthority())
	{
		return;
	}

	for (const TSubclassOf<UGameplayAbility>& StartupAbility : StartupAbilities)
	{
		if (StartupAbility)
		{
			AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(StartupAbility, 1));
		}
	}

	bStartupAbilitiesGranted = true;
}

void ARDBasecharacter::HandleHealthChanged(const FOnAttributeChangeData& ChangeData)
{
	CurrentHealth = ChangeData.NewValue;
}

void ARDBasecharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ARDBasecharacter::Move);
		}

		if (LookAction)
		{
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ARDBasecharacter::Look);
		}

		if (JumpAction)
		{
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ARDBasecharacter::StartJump);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ARDBasecharacter::StopJump);
		}

		if (LockOnAction)
		{
			EnhancedInputComponent->BindAction(LockOnAction, ETriggerEvent::Started, this, &ARDBasecharacter::ToggleLockOn);
		}

		if (SlideAction)
		{
			EnhancedInputComponent->BindAction(SlideAction, ETriggerEvent::Started, this, &ARDBasecharacter::HandleSlideInput);
		}

		if (SwitchTargetAction)
		{
			EnhancedInputComponent->BindAction(SwitchTargetAction, ETriggerEvent::Triggered, this, &ARDBasecharacter::SwitchTarget);
		}
	}
}

void ARDBasecharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();
	if (!Controller) return;

	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	if (MovementVector.Y != 0.f)
	{
		AddMovementInput(ForwardDirection, bIsSliding ? MovementVector.Y * SlideSteeringControl : MovementVector.Y);
	}

	if (MovementVector.X != 0.f)
	{
		AddMovementInput(RightDirection, bIsSliding ? MovementVector.X * SlideSteeringControl : MovementVector.X);
	}

	if (bIsSliding)
	{
		FVector InputDirection = (ForwardDirection * MovementVector.Y) + (RightDirection * MovementVector.X);
		InputDirection.Z = 0.f;
		if (!InputDirection.IsNearlyZero())
		{
			SlideDirection = FMath::Lerp(SlideDirection, InputDirection.GetSafeNormal(), SlideSteeringControl).GetSafeNormal();
		}
	}
}

void ARDBasecharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();
	if (!Controller) return;

	if (TargetingCameraComponent)
	{
		TargetingCameraComponent->HandleLookInput(LookAxisVector);
	}
}

void ARDBasecharacter::StartJump(const FInputActionValue& Value)
{
	if (bIsSliding)
	{
		return;
	}

	Jump();
}

void ARDBasecharacter::StopJump(const FInputActionValue& Value)
{
	StopJumping();
}

void ARDBasecharacter::HandleSlideInput(const FInputActionValue& Value)
{
	bHasSlideInputQueued = true;
	StartSlide();
	bHasSlideInputQueued = false;
}

void ARDBasecharacter::ToggleLockOn(const FInputActionValue& Value)
{
	if (TargetingCameraComponent)
	{
		TargetingCameraComponent->ToggleLockOn();
	}
}

void ARDBasecharacter::SwitchTarget(const FInputActionValue& Value)
{
	if (TargetingCameraComponent)
	{
		TargetingCameraComponent->SwitchTarget(Value.Get<float>());
	}
}

bool ARDBasecharacter::StartSlide()
{
	if (!CanStartSlide())
	{
		return false;
	}

	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (!MovementComponent)
	{
		return false;
	}

	const FVector CurrentVelocity = GetVelocity();
	FVector HorizontalVelocity = FVector(CurrentVelocity.X, CurrentVelocity.Y, 0.f);
	const float CurrentSpeed = HorizontalVelocity.Size();
	HorizontalSpeed = CurrentSpeed;

	SlideDirection = CurrentSpeed > KINDA_SMALL_NUMBER ? HorizontalVelocity.GetSafeNormal() : GetActorForwardVector().GetSafeNormal2D();
	if (SlideDirection.IsNearlyZero())
	{
		SlideDirection = FVector::ForwardVector;
	}

	bIsSliding = true;
	bPendingSlideUnCrouch = false;
	SlideTimeRemaining = SlideDuration;
	SlideCooldownRemaining = SlideCooldown;

	MovementComponent->SetCrouchedHalfHeight(GetClampedSlideCapsuleHalfHeight());
	Crouch();

	MovementComponent->GroundFriction = SlideGroundFriction;
	MovementComponent->BrakingDecelerationWalking = SlideBrakingDeceleration;
	MovementComponent->MaxWalkSpeed = FMath::Max(DefaultMaxWalkSpeed, SlideMaxSpeed);
	MovementComponent->bOrientRotationToMovement = false;

	const float StartSpeed = FMath::Clamp(FMath::Max(CurrentSpeed, SlideMinStartSpeed) + SlideInitialSpeedBoost, 0.f, SlideMaxSpeed);
	MovementComponent->Velocity = SlideDirection * StartSpeed + FVector(0.f, 0.f, CurrentVelocity.Z);
	HorizontalSpeed = StartSpeed;
	return true;
}

void ARDBasecharacter::StopSlide()
{
	if (!bIsSliding)
	{
		return;
	}

	bIsSliding = false;
	bPendingSlideUnCrouch = true;
	SlideTimeRemaining = 0.f;

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->GroundFriction = DefaultGroundFriction;
		MovementComponent->BrakingDecelerationWalking = DefaultBrakingDecelerationWalking;
		MovementComponent->MaxWalkSpeed = DefaultMaxWalkSpeed;
	}

	UnCrouch();
	if (!bIsCrouched)
	{
		bPendingSlideUnCrouch = false;
	}
}

void ARDBasecharacter::UpdateWallRunState(float DeltaSeconds)
{
	const bool bShouldClearWallJumpRequestAtEnd = bWallJumpRequested && !bIsWallRunning;

	if (WallJumpAnimFlagRemaining > 0.f)
	{
		WallJumpAnimFlagRemaining -= DeltaSeconds;
		if (WallJumpAnimFlagRemaining <= 0.f)
		{
			SetWallJumpTriggered(false);
		}
	}

	CleanupInvalidWallRunSurfaces();

	if (bIsWallRunning)
	{
		if (!CurrentWallRunSurface.IsValid() || !HasOverlappingWallRunSurface(CurrentWallRunSurface.Get()))
		{
			const float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
			if ((CurrentTime - WallRunLastWallContactTime) > LostWallForgivenessTime)
			{
				StopWallRun(false);
			}
		}
		return;
	}

	const UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (!MovementComponent || bIsSliding || MovementComponent->MovementMode != MOVE_Falling)
	{
		if (bShouldClearWallJumpRequestAtEnd)
		{
			bWallJumpRequested = false;
		}
		return;
	}

	TryStartWallRun();

	if (bShouldClearWallJumpRequestAtEnd)
	{
		bWallJumpRequested = false;
	}
}

bool ARDBasecharacter::TryStartWallRun()
{
	if (UWallRunSurfaceComponent* Surface = FindBestWallRunSurface())
	{
		if (CanStartWallRunOnSurface(Surface))
		{
			StartWallRun(Surface);
			return true;
		}
	}
	else
	{
		DebugWallRunMessage(TEXT("WallRun: No valid surface candidate"), FColor::Orange);
	}

	return false;
}

bool ARDBasecharacter::CanStartWallRunOnSurface(UWallRunSurfaceComponent* Surface) const
{
	const UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (!MovementComponent || !Surface || bIsWallRunning || bIsSliding)
	{
		DebugWallRunMessage(TEXT("WallRun Blocked: invalid state or surface"), FColor::Orange);
		return false;
	}

	if (MovementComponent->MovementMode != MOVE_Falling || !Surface->IsWallRunEnabled())
	{
		DebugWallRunMessage(TEXT("WallRun Blocked: not falling or surface disabled"), FColor::Orange);
		return false;
	}

	const float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	if (CurrentTime < WallJumpBlockedUntilTime)
	{
		if (!LastWallJumpSurface.IsValid() || Surface == LastWallJumpSurface.Get())
		{
			DebugWallRunMessage(TEXT("WallRun Blocked: wall jump reattach block active"), FColor::Orange);
			return false;
		}
	}

	if ((CurrentTime - LastJumpPressedTime) > JumpToWallGraceTime)
	{
		DebugWallRunMessage(
			FString::Printf(TEXT("WallRun Blocked: jump grace expired (Current: %.2f, Allowed: %.2f)"), CurrentTime - LastJumpPressedTime, JumpToWallGraceTime),
			FColor::Orange);
		return false;
	}

	const float HorizontalSpeed2D = FVector(GetVelocity().X, GetVelocity().Y, 0.f).Size();
	if (HorizontalSpeed2D < MinSpeedToStartWallRun)
	{
		DebugWallRunMessage(
			FString::Printf(TEXT("WallRun Blocked: speed too low (Current: %.2f, Required: %.2f)"), HorizontalSpeed2D, MinSpeedToStartWallRun),
			FColor::Orange);
		return false;
	}

	const FVector WallNormal = Surface->GetWallNormal();
	const float WallAngle = GetWallAngleFromUpDeg(WallNormal);
	if (WallAngle < MinWallAngleFromUpDeg || WallAngle > MaxWallAngleFromUpDeg)
	{
		DebugWallRunMessage(
			FString::Printf(TEXT("Wall Angle Error! Current: %.2f, Allowed: %.2f - %.2f"), WallAngle, MinWallAngleFromUpDeg, MaxWallAngleFromUpDeg),
			FColor::Red);
		return false;
	}

	const FVector WallRunDirection = Surface->GetWallRunDirectionFromVelocity(GetVelocity());
	if (WallRunDirection.IsNearlyZero() || !Surface->IsDirectionAllowed(WallRunDirection))
	{
		DebugWallRunMessage(TEXT("WallRun Blocked: invalid wall run direction"), FColor::Red);
		return false;
	}

	const float ApproachAngle = GetWallApproachAngleDeg(WallNormal);
	if (ApproachAngle < MinApproachAngleDeg || ApproachAngle > MaxApproachAngleDeg)
	{
		DebugWallRunMessage(
			FString::Printf(TEXT("Angle Error! Current: %.2f, Allowed: %.2f - %.2f"), ApproachAngle, MinApproachAngleDeg, MaxApproachAngleDeg),
			FColor::Red);
		return false;
	}

	DebugWallRunMessage(TEXT("WallRun Ready: all conditions passed"), FColor::Green);
	return true;
}

UWallRunSurfaceComponent* ARDBasecharacter::FindBestWallRunSurface() const
{
	const FVector HorizontalVelocity = FVector(GetVelocity().X, GetVelocity().Y, 0.f).GetSafeNormal();

	UWallRunSurfaceComponent* BestSurface = nullptr;
	float BestScore = -FLT_MAX;

	for (const TWeakObjectPtr<UWallRunSurfaceComponent>& SurfacePtr : OverlappingWallRunSurfaces)
	{
		UWallRunSurfaceComponent* Surface = SurfacePtr.Get();
		if (!Surface || !Surface->IsWallRunEnabled())
		{
			continue;
		}

		const FVector CandidateDirection = Surface->GetWallRunDirectionFromVelocity(GetVelocity());
		if (CandidateDirection.IsNearlyZero() || !Surface->IsDirectionAllowed(CandidateDirection))
		{
			continue;
		}

		const FVector ClosestPoint = Surface->GetClosestPointToLocation(GetActorLocation());
		const float DistanceScore = -FVector::DistSquared2D(GetActorLocation(), ClosestPoint) * 0.001f;
		const float DirectionScore = HorizontalVelocity.IsNearlyZero() ? 0.f : FVector::DotProduct(HorizontalVelocity, CandidateDirection) * 1000.f;
		const float ApproachScore = -GetWallApproachAngleDeg(Surface->GetWallNormal()) * 2.f;
		const float Score = DirectionScore + DistanceScore + ApproachScore;

		if (Score > BestScore)
		{
			BestScore = Score;
			BestSurface = Surface;
		}
	}

	return BestSurface;
}

void ARDBasecharacter::StartWallRun(UWallRunSurfaceComponent* Surface)
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (!MovementComponent || !Surface)
	{
		return;
	}

	CurrentWallRunSurface = Surface;
	CurrentWallNormal = Surface->GetWallNormal();
	CurrentWallRunDirection = Surface->GetWallRunDirectionFromVelocity(GetVelocity());
	CurrentWallRunSide = DetermineWallRunSide(Surface, CurrentWallRunDirection);

	const float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	WallRunStartTime = CurrentTime;
	WallRunLastWallContactTime = CurrentTime;
	bIsWallRunning = true;
	bWallJumpRequested = false;

	const float StartSpeed = FMath::Max(MinSpeedToMaintainWallRun, FVector(GetVelocity().X, GetVelocity().Y, 0.f).Size());
	MovementComponent->Velocity = CurrentWallRunDirection * StartSpeed + FVector(0.f, 0.f, FMath::Max(GetVelocity().Z, 0.f));
	MovementComponent->SetMovementMode(MOVE_Custom, static_cast<uint8>(ERDBasecharacterCustomMovementMode::CMOVE_WallRun));
	MovementComponent->bOrientRotationToMovement = false;
}

void ARDBasecharacter::StopWallRun(bool bLaunchAway)
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (!MovementComponent)
	{
		return;
	}

	const bool bWasWallRunning = bIsWallRunning || IsWallRunMovementMode();
	if (!bWasWallRunning)
	{
		return;
	}

	const FVector WallNormalToUse = CurrentWallNormal;
	ClearWallRunState();
	MovementComponent->SetMovementMode(MOVE_Falling);

	if (bLaunchAway && !WallNormalToUse.IsNearlyZero())
	{
		MovementComponent->Velocity += WallNormalToUse.GetSafeNormal() * (WallJumpHorizontalStrength * 0.2f);
	}
}

void ARDBasecharacter::PerformWallJump()
{
	if (!CanPerformWallJump())
	{
		return;
	}

	LastWallJumpSide = CurrentWallRunSide;
	LastWallJumpSurface = CurrentWallRunSurface;
	WallJumpBlockedUntilTime = (GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f) + WallJumpReattachBlockTime;

	const FVector LaunchVelocity =
		(CurrentWallNormal.GetSafeNormal() * WallJumpHorizontalStrength) +
		(FVector::UpVector * WallJumpVerticalStrength) +
		(GetActorForwardVector().GetSafeNormal2D() * WallJumpForwardBoost);

	StopWallRun(false);
	LaunchCharacter(LaunchVelocity, true, true);
	SetWallJumpTriggered(true);
}

void ARDBasecharacter::OnWallRunDetectorBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	(void)OverlappedComponent;
	(void)OtherActor;
	(void)OtherBodyIndex;
	(void)bFromSweep;
	(void)SweepResult;

	UWallRunSurfaceComponent* Surface = Cast<UWallRunSurfaceComponent>(OtherComp);
	if (!Surface)
	{
		return;
	}

	DebugWallRunMessage(TEXT("Overlapped!"), FColor::Yellow);

	const UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	const float HorizontalSpeed2D = FVector(GetVelocity().X, GetVelocity().Y, 0.f).Size();
	if (MovementComponent && MovementComponent->MovementMode == MOVE_Falling && HorizontalSpeed2D >= MinSpeedToStartWallRun)
	{
		DebugWallRunMessage(TEXT("Character Falling!"), FColor::Cyan);

		const FVector WallNormal = Surface->GetWallNormal();
		const FVector WallRunDirection = Surface->GetWallRunDirectionFromVelocity(GetVelocity());
		const float ApproachAngle = GetWallApproachAngleDeg(WallNormal);
		const bool bAngleValid = ApproachAngle >= MinApproachAngleDeg && ApproachAngle <= MaxApproachAngleDeg;
		if (!bAngleValid)
		{
			DebugWallRunMessage(
				FString::Printf(TEXT("Angle Error! Current: %.2f, Allowed: %.2f - %.2f"), ApproachAngle, MinApproachAngleDeg, MaxApproachAngleDeg),
				FColor::Red);
		}
	}

	AddOverlappingWallRunSurface(Surface);
}

void ARDBasecharacter::OnWallRunDetectorEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	(void)OverlappedComponent;
	(void)OtherActor;
	(void)OtherBodyIndex;

	UWallRunSurfaceComponent* Surface = Cast<UWallRunSurfaceComponent>(OtherComp);
	if (!Surface)
	{
		return;
	}

	RemoveOverlappingWallRunSurface(Surface);
}

void ARDBasecharacter::AddOverlappingWallRunSurface(UWallRunSurfaceComponent* Surface)
{
	if (!Surface)
	{
		return;
	}

	for (const TWeakObjectPtr<UWallRunSurfaceComponent>& SurfacePtr : OverlappingWallRunSurfaces)
	{
		if (SurfacePtr.Get() == Surface)
		{
			return;
		}
	}

	OverlappingWallRunSurfaces.Add(Surface);
}

void ARDBasecharacter::RemoveOverlappingWallRunSurface(UWallRunSurfaceComponent* Surface)
{
	OverlappingWallRunSurfaces.RemoveAll([Surface](const TWeakObjectPtr<UWallRunSurfaceComponent>& SurfacePtr)
	{
		return !SurfacePtr.IsValid() || SurfacePtr.Get() == Surface;
	});
}

bool ARDBasecharacter::HasOverlappingWallRunSurface(UWallRunSurfaceComponent* Surface) const
{
	if (!Surface)
	{
		return false;
	}

	for (const TWeakObjectPtr<UWallRunSurfaceComponent>& SurfacePtr : OverlappingWallRunSurfaces)
	{
		if (SurfacePtr.Get() == Surface)
		{
			return true;
		}
	}

	return false;
}

void ARDBasecharacter::CleanupInvalidWallRunSurfaces()
{
	OverlappingWallRunSurfaces.RemoveAll([](const TWeakObjectPtr<UWallRunSurfaceComponent>& SurfacePtr)
	{
		return !SurfacePtr.IsValid() || !SurfacePtr->IsWallRunEnabled();
	});
}

FVector ARDBasecharacter::ComputeWallRunDirection(const FVector& WallNormal, const FVector& ReferenceDirection) const
{
	const FVector HorizontalWallNormal = FVector(WallNormal.X, WallNormal.Y, 0.f).GetSafeNormal();
	FVector AlongWall = FVector::CrossProduct(FVector::UpVector, HorizontalWallNormal).GetSafeNormal();
	if (AlongWall.IsNearlyZero())
	{
		return GetActorForwardVector().GetSafeNormal2D();
	}

	FVector Reference = FVector(ReferenceDirection.X, ReferenceDirection.Y, 0.f).GetSafeNormal();
	if (Reference.IsNearlyZero())
	{
		Reference = GetActorForwardVector().GetSafeNormal2D();
	}

	if (FVector::DotProduct(AlongWall, Reference) < 0.f)
	{
		AlongWall *= -1.f;
	}

	return AlongWall;
}

EWallRunSide ARDBasecharacter::DetermineWallRunSide(const UWallRunSurfaceComponent* Surface, const FVector& WallRunDirection) const
{
	if (!Surface)
	{
		return EWallRunSide::None;
	}

	const FVector ToSurface = (Surface->GetClosestPointToLocation(GetActorLocation()) - GetActorLocation()).GetSafeNormal2D();
	const FVector AlongWallDirection = WallRunDirection.GetSafeNormal2D();
	if (ToSurface.IsNearlyZero() || AlongWallDirection.IsNearlyZero())
	{
		return EWallRunSide::None;
	}

	const float CrossZ = FVector::CrossProduct(AlongWallDirection, ToSurface).Z;
	return CrossZ >= 0.f ? EWallRunSide::Right : EWallRunSide::Left;
}

float ARDBasecharacter::GetWallAngleFromUpDeg(const FVector& WallNormal) const
{
	const float Dot = FVector::DotProduct(WallNormal.GetSafeNormal(), FVector::UpVector);
	return FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(Dot, -1.f, 1.f)));
}

float ARDBasecharacter::GetWallApproachAngleDeg(const FVector& WallNormal) const
{
	const FVector HorizontalForward = GetActorForwardVector().GetSafeNormal2D();
	if (HorizontalForward.IsNearlyZero())
	{
		return 0.f;
	}

	const FVector IntoWallDirection = FVector(-WallNormal.X, -WallNormal.Y, 0.f).GetSafeNormal();
	if (IntoWallDirection.IsNearlyZero())
	{
		return 0.f;
	}

	const float Dot = FVector::DotProduct(HorizontalForward, IntoWallDirection);
	return FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(Dot, -1.f, 1.f)));
}

bool ARDBasecharacter::CanPerformWallJump() const
{
	return bEnableWallJump
		&& bIsWallRunning
		&& IsWallRunMovementMode()
		&& CurrentWallRunSurface.IsValid()
		&& !CurrentWallNormal.IsNearlyZero()
		&& !CurrentWallRunDirection.IsNearlyZero();
}

float ARDBasecharacter::GetWallRunTargetSpeed() const
{
	return CurrentWallRunSurface.IsValid()
		? CurrentWallRunSurface->ResolveWallRunSpeed(WallRunSpeed)
		: WallRunSpeed;
}

float ARDBasecharacter::GetWallRunMaxDurationForCurrentSurface() const
{
	return CurrentWallRunSurface.IsValid()
		? CurrentWallRunSurface->ResolveWallRunMaxDuration(WallRunMaxDuration)
		: WallRunMaxDuration;
}

bool ARDBasecharacter::IsWallRunMovementMode() const
{
	const UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	return MovementComponent
		&& MovementComponent->MovementMode == MOVE_Custom
		&& MovementComponent->CustomMovementMode == static_cast<uint8>(ERDBasecharacterCustomMovementMode::CMOVE_WallRun);
}

void ARDBasecharacter::ClearWallRunState()
{
	bIsWallRunning = false;
	CurrentWallNormal = FVector::ZeroVector;
	CurrentWallRunDirection = FVector::ZeroVector;
	CurrentWallRunSide = EWallRunSide::None;
	CurrentWallRunSurface = nullptr;
	WallRunLastWallContactTime = -1000.f;
}

void ARDBasecharacter::SetWallJumpTriggered(bool bNewTriggered)
{
	bWallJumpTriggered = bNewTriggered;
	WallJumpAnimFlagRemaining = bNewTriggered ? WallJumpAnimFlagDuration : 0.f;
	if (!bNewTriggered)
	{
		LastWallJumpSide = EWallRunSide::None;
	}
}

void ARDBasecharacter::DebugWallRunMessage(const FString& Message, const FColor& Color) const
{
	if (!bDebugWallRun || !GetWorld())
	{
		return;
	}

	const float CurrentTime = GetWorld()->GetTimeSeconds();
	if ((CurrentTime - LastWallRunDebugTime) < 0.08f)
	{
		return;
	}

	LastWallRunDebugTime = CurrentTime;

	UE_LOG(LogTemp, Log, TEXT("%s"), *Message);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, Color, Message);
	}
}

URDBasecharacterMovementComponent* ARDBasecharacter::GetRDBasecharacterMovementComponent() const
{
	return Cast<URDBasecharacterMovementComponent>(GetCharacterMovement());
}

void ARDBasecharacter::UpdateCameraSystem(float DeltaSeconds)
{
	if (TargetingCameraComponent)
	{
		TargetingCameraComponent->TickCamera(DeltaSeconds);
	}

	UpdateCharacterFacing(DeltaSeconds);
}

void ARDBasecharacter::UpdateSlide(float DeltaSeconds)
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (!MovementComponent)
	{
		return;
	}

	MovementComponent->SetCrouchedHalfHeight(GetClampedSlideCapsuleHalfHeight());

	if (!bIsSliding)
	{
		if (bPendingSlideUnCrouch && bIsCrouched)
		{
			UnCrouch();
		}

		if (!bIsCrouched)
		{
			bPendingSlideUnCrouch = false;
		}

		return;
	}

	if (!MovementComponent->IsMovingOnGround())
	{
		StopSlide();
		return;
	}

	SlideTimeRemaining -= DeltaSeconds;

	FVector HorizontalVelocity = FVector(MovementComponent->Velocity.X, MovementComponent->Velocity.Y, 0.f);
	float CurrentHorizontalSpeed = HorizontalVelocity.Size();
	HorizontalSpeed = CurrentHorizontalSpeed;

	if (CurrentHorizontalSpeed > KINDA_SMALL_NUMBER)
	{
		SlideDirection = HorizontalVelocity.GetSafeNormal();
	}

	CurrentHorizontalSpeed = FMath::Clamp(CurrentHorizontalSpeed + SlideAcceleration * DeltaSeconds, 0.f, SlideMaxSpeed);
	MovementComponent->Velocity = SlideDirection * CurrentHorizontalSpeed + FVector(0.f, 0.f, MovementComponent->Velocity.Z);
	HorizontalSpeed = CurrentHorizontalSpeed;

	if (SlideTimeRemaining <= 0.f || CurrentHorizontalSpeed <= SlideMinSpeedToEnd)
	{
		StopSlide();
	}
}

void ARDBasecharacter::UpdateCharacterFacing(float DeltaSeconds)
{
	if (bIsWallRunning)
	{
		GetCharacterMovement()->bOrientRotationToMovement = false;

		if (!CurrentWallRunDirection.IsNearlyZero())
		{
			FRotator DesiredActorRot = CurrentWallRunDirection.Rotation();
			DesiredActorRot.Pitch = 0.f;
			DesiredActorRot.Roll = 0.f;
			SetActorRotation(FMath::RInterpTo(GetActorRotation(), DesiredActorRot, DeltaSeconds, WallRunRotationInterpSpeed));
		}

		return;
	}

	if (bIsSliding)
	{
		GetCharacterMovement()->bOrientRotationToMovement = false;

		if (!SlideDirection.IsNearlyZero())
		{
			FRotator DesiredActorRot = SlideDirection.Rotation();
			DesiredActorRot.Pitch = 0.f;
			DesiredActorRot.Roll = 0.f;
			SetActorRotation(FMath::RInterpTo(GetActorRotation(), DesiredActorRot, DeltaSeconds, 12.f));
		}

		return;
	}

	AActor* CurrentLockTarget = TargetingCameraComponent ? TargetingCameraComponent->GetCurrentLockTarget() : nullptr;
	if (TargetingCameraComponent
		&& TargetingCameraComponent->GetCameraState() == ECombatCameraState::LockOn
		&& CurrentLockTarget)
	{
		GetCharacterMovement()->bOrientRotationToMovement = false;

		const FVector ToTarget = TargetingCameraComponent->GetTargetPivot(CurrentLockTarget) - GetActorLocation();
		FRotator DesiredActorRot = ToTarget.Rotation();
		DesiredActorRot.Pitch = 0.f;
		DesiredActorRot.Roll = 0.f;
		SetActorRotation(FMath::RInterpTo(GetActorRotation(), DesiredActorRot, DeltaSeconds, 10.f));
	}
	else
	{
		GetCharacterMovement()->bOrientRotationToMovement = true;
	}
}

bool ARDBasecharacter::CanStartSlide() const
{
	if (!bSlideEnabled)
	{
		return false;
	}

	const UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (!MovementComponent || !Controller || bIsSliding || SlideCooldownRemaining > 0.f)
	{
		return false;
	}

	if (!MovementComponent->IsMovingOnGround())
	{
		return false;
	}

	const float CurrentHorizontalSpeed = FVector(GetVelocity().X, GetVelocity().Y, 0.f).Size();
	return bCanSlideFromStandingStill || CurrentHorizontalSpeed >= SlideTriggerMinSpeed;
}

float ARDBasecharacter::GetClampedSlideCapsuleHalfHeight() const
{
	const float CapsuleRadius = GetCapsuleComponent() ? GetCapsuleComponent()->GetUnscaledCapsuleRadius() : 0.f;
	return FMath::Max(SlideCapsuleHalfHeight, CapsuleRadius + 1.f);
}

float ARDBasecharacter::GetHorizontalSpeed() const
{
	return HorizontalSpeed;
}

FVector ARDBasecharacter::GetSlideDirection() const
{
	return SlideDirection;
}

bool ARDBasecharacter::IsWallRunning() const
{
	return bIsWallRunning;
}

bool ARDBasecharacter::IsWallJumping() const
{
	return bWallJumpTriggered;
}

bool ARDBasecharacter::WasWallJumpRequestedThisFrame() const
{
	return bWallJumpRequested;
}

EWallRunSide ARDBasecharacter::GetWallJumpSide() const
{
	return LastWallJumpSide;
}

EWallRunSide ARDBasecharacter::GetWallRunSide() const
{
	return CurrentWallRunSide;
}

bool ARDBasecharacter::IsWallRunningOnLeft() const
{
	return bIsWallRunning && CurrentWallRunSide == EWallRunSide::Left;
}

bool ARDBasecharacter::IsWallRunningOnRight() const
{
	return bIsWallRunning && CurrentWallRunSide == EWallRunSide::Right;
}

bool ARDBasecharacter::IsWallJumpFromLeft() const
{
	return bWallJumpTriggered && LastWallJumpSide == EWallRunSide::Left;
}

bool ARDBasecharacter::IsWallJumpFromRight() const
{
	return bWallJumpTriggered && LastWallJumpSide == EWallRunSide::Right;
}

FVector ARDBasecharacter::GetWallRunDirection() const
{
	return CurrentWallRunDirection;
}

FVector ARDBasecharacter::GetWallNormal() const
{
	return CurrentWallNormal;
}

float ARDBasecharacter::GetWallRunSpeed() const
{
	return FMath::Abs(FVector::DotProduct(GetVelocity(), CurrentWallRunDirection.GetSafeNormal()));
}

AActor* ARDBasecharacter::GetCurrentLockTarget() const
{
	return TargetingCameraComponent ? TargetingCameraComponent->GetCurrentLockTarget() : nullptr;
}

AActor* ARDBasecharacter::GetPreviousLockTarget() const
{
	return TargetingCameraComponent ? TargetingCameraComponent->GetPreviousLockTarget() : nullptr;
}

ECombatCameraState ARDBasecharacter::GetCombatCameraState() const
{
	return TargetingCameraComponent ? TargetingCameraComponent->GetCameraState() : ECombatCameraState::Explore;
}

ELockOnZone ARDBasecharacter::GetCurrentLockOnZone() const
{
	return TargetingCameraComponent ? TargetingCameraComponent->GetCurrentZone() : ELockOnZone::Center;
}
