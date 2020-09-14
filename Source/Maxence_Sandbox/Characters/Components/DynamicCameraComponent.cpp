// Fill out your copyright notice in the Description page of Project Settings.


#include "DynamicCameraComponent.h"

#include <Camera/CameraComponent.h>
#include <GameFramework/SpringArmComponent.h>
#include <Components/SphereComponent.h>
#include <Characters/Interfaces/Targetable.h>

#include <Runtime/Engine/Classes/Kismet/KismetMathLibrary.h>
#include <Runtime/Engine/Classes/Kismet/GameplayStatics.h>

#include <Characters/Maxence_SandboxCharacter.h>

#include <typeinfo>
#include <typeindex>

// Sets default values
UDynamicCameraComponent::UDynamicCameraComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	/** CAMERA INITIALISATION */
	// Properties
	BaseTurnRate = 1.f;
	BaseLookUpRate = 1.f;
	MaxPitchAngle = -30.f;
	TargetLocked = false;
	RotationInterpSpeed = 5.f;
	FocusInterpSpeed = 2.f;
	MinimumRangeToSelect = 5000;
	DistanceCameraWhenUnlocked = 350;
	DistanceCameraWhenLocked = 500;
	MaxPitchAngle = 25;
	MinPitchAngle = -25;
	ForceReset = false;
	NavigateThreshold = 0.35f;
	prevNavIncrementSign = 0;
	bCyclicNavigation = true;
	bNavigateOnlyVisible = true;
	NavigationRaycastOffset = FVector(0.0f, 0.0f, 70.f);
	MaxAngleNavigation = 100.f;
	FacingAngleNotReseting = 1.33f;
	AnchorLocked = FVector(0, 50, 50);
	PositionOffsetFree = FVector(0, 0, 25);
	/***/
	TargetArmLength = 350.f; // The camera follows at this distance behind the character	
	bUsePawnControlRotation = true; // Rotate the arm based on the controller

	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.0f;
	YAxisDirection = 1;

	ResetCameraRate = 1.f;
	ResetCurrentTime = 0.f;
	TimeBeforeReset = 2.f;

	// Create camera component.
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(this, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	//Potentially do a eclipse instead of a perfect circle
	CameraRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CameraRangeSphere"));
	CameraRangeSphere->SetupAttachment(Camera);
	CameraRangeSphere->SetCollisionProfileName("Trigger");
	CameraRangeSphere->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECollisionResponse::ECR_Ignore);
	CameraRangeSphere->SetCollisionResponseToChannel(ECC_GameTraceChannel4, ECollisionResponse::ECR_Ignore);

	//May behave oddly move to BeginPlay as soon as the spawner are done
	CameraRangeSphere->OnComponentBeginOverlap.AddDynamic(this, &UDynamicCameraComponent::OnObjectEntersRange);
	CameraRangeSphere->OnComponentEndOverlap.AddDynamic(this, &UDynamicCameraComponent::OnObjectLeavesRange);
}

void UDynamicCameraComponent::BeginPlay()
{
	Super::BeginPlay();

	CameraRangeSphere->SetSphereRadius(MinimumRangeToSelect);

	SetModeFree(CameraStates::CVOID);
}

void UDynamicCameraComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	CameraRangeSphere->SetSphereRadius(MinimumRangeToSelect);
	CameraRangeSphere->OnComponentBeginOverlap.RemoveDynamic(this, &UDynamicCameraComponent::OnObjectEntersRange);
	CameraRangeSphere->OnComponentEndOverlap.RemoveDynamic(this, &UDynamicCameraComponent::OnObjectLeavesRange);
}

// Called every frame
void UDynamicCameraComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction)
{
	DoActionCamera.ExecuteIfBound();

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}


#pragma region CAMERA

void UDynamicCameraComponent::HandlingFinishedState(CameraStates PrevState)
{
	switch (PrevState)
	{
	case CameraStates::LOCKED:
		EndModeLocked();
		break;
	case CameraStates::FREE:
		EndModeFree();
		UGameplayStatics::GetPlayerCameraManager(this, 0)->ViewPitchMax = MAX_FLT;
		UGameplayStatics::GetPlayerCameraManager(this, 0)->ViewPitchMin = -MAX_FLT;
		break;
	default:
		return;
	}
}

void UDynamicCameraComponent::OnObjectEntersRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Other Actor is the actor that triggered the event. Check that is not ourself
	if ((OtherActor != nullptr) && (OtherActor != GetOwner()) && (OtherComp != nullptr) && (Cast<ITargetable>(OtherActor)))
	{
		ObjectsInRange.AddUnique(OtherActor);
	}
}

void UDynamicCameraComponent::OnObjectLeavesRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// Other Actor is the actor that triggered the event. Check that is not ourself.  
	if ((OtherActor != nullptr) && (OtherActor != GetOwner()) && (OtherComp != nullptr) && (ObjectsInRange.Contains(OtherActor)))
	{
		ObjectsInRange.RemoveSingle(OtherActor);
	}
}


void UDynamicCameraComponent::PreventResetCamera(bool _HardStop)
{
	if (_HardStop)
		ForceReset = false;

	ResetCurrentTime = 0;
	IsCameraReseting = false;
}

void UDynamicCameraComponent::ResetCamera(float DeltaSeconds)
{
	AMaxence_SandboxCharacter* Player = Cast<AMaxence_SandboxCharacter>(GetOwner());

	float angleBetweenForwards = FMath::Acos(FVector::DotProduct(Camera->GetForwardVector(), Player->GetActorForwardVector()));
	if (!ForceReset)
	{
		if (FMath::Abs(angleBetweenForwards) > FacingAngleNotReseting)
		{
			PreventResetCamera(false);
			return;
		}
		if (!IsCameraReseting)
		{
			ResetCurrentTime += DeltaSeconds;
			if (ResetCurrentTime >= TimeBeforeReset)
				IsCameraReseting = true;
			return;
		}
	}

	//if(Player->GetLocomotion()->GetRotationMode() == ELocomotionRotationMode::LRM_LookingDirection)
	//	targetRotation = ((targetRotation + Player->GetLocomotion()->GetLookingRotation()) * 0.5f);

	FRotator targetRotation = UKismetMathLibrary::FindLookAtRotation(GetComponentLocation(), Player->GetMesh()->GetSocketLocation(LookAtCameraBone));
	targetRotation.Yaw = Player->GetActorRotation().Yaw;

	FRotator deltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(targetRotation, Camera->GetComponentRotation());
	Player->AddControllerYawInput(deltaRotation.Yaw * DeltaSeconds * ResetCameraRate);
	Player->AddControllerPitchInput(deltaRotation.Pitch * DeltaSeconds * ResetCameraRate * -1.f);

	// Stop camera reset when reset is complete
	// TODO should add reset threshold ?
	if (deltaRotation.IsNearlyZero(1.f))
		PreventResetCamera(true);
}

void UDynamicCameraComponent::SetModeFree(CameraStates PrevState)
{
	HandlingFinishedState(PrevState);

	UGameplayStatics::GetPlayerCameraManager(this, 0)->ViewPitchMax = MaxPitchAngle;
	UGameplayStatics::GetPlayerCameraManager(this, 0)->ViewPitchMin = MinPitchAngle;

	DoActionCamera.BindUFunction(this, FName("DoActionFree"));

	OnCameraChangeTarget.Broadcast(nullptr);
	OnCameraUnlock.Broadcast();

	CurrentTarget = nullptr;
}

void UDynamicCameraComponent::SetModeLocked(CameraStates PrevState)
{
	HandlingFinishedState(PrevState);

	ClosestTargetDistance = MinimumRangeToSelect;

	float distance = 0.f;
	AActor* newTarget = nullptr;

	FHitResult HitInfos;

	// Select target within range
	for (AActor* actor : ObjectsInRange)
	{
		
		// Instant select valid target.
		if (bNavigateOnlyVisible)
		{
			// Check visibility
			if (GetWorld()->LineTraceSingleByChannel(HitInfos, Camera->GetComponentLocation() + NavigationRaycastOffset, actor->GetActorLocation(), ECollisionChannel::ECC_Visibility)
				&& HitInfos.Actor.Get() != actor)
			{
				continue;
			}
		}

		distance = GetOwner()->GetDistanceTo(actor);
		if (distance < ClosestTargetDistance)
		{
			ClosestTargetDistance = distance;
			newTarget = actor;
			UE_LOG(LogTemp, Warning, TEXT("Distance from current Target: %f"), distance);
		}
	}

	if (newTarget != nullptr)
	{
		TargetLocked = true;
		DoActionCamera.BindUFunction(this, FName("DoActionLocked"));

		SetCurrentTarget(newTarget);
	}
	else
	{
		SetModeFree(CameraStates::LOCKED);
	}
}

void UDynamicCameraComponent::DoActionLocked()
{
	if (!TargetLocked || ObjectsInRange.Num() == 0)
	{
		SetModeFree(CameraStates::LOCKED);
		return;
	}

	AActor* owner = GetOwner();
	check(owner != nullptr);

	//LockCameraPosition(SocketOffset, TargetArmLength);

	FRotator newRotation = FRotator();
	LookAt(newRotation);
	owner->GetInstigatorController()->SetControlRotation(newRotation);
}

void UDynamicCameraComponent::DoActionFree()
{
	// Update spring arm data
	TargetArmLength = UKismetMathLibrary::FInterpTo(TargetArmLength, DistanceCameraWhenUnlocked, GetWorld()->GetDeltaSeconds(), RotationInterpSpeed);
	SocketOffset = UKismetMathLibrary::VInterpTo(SocketOffset, PositionOffsetFree, GetWorld()->GetDeltaSeconds(), RotationInterpSpeed);

	//DynamicPositionning();
	ResetCamera(GetWorld()->DeltaTimeSeconds);
}

void UDynamicCameraComponent::EndModeFree()
{
	DoActionCamera.Unbind();
}

void UDynamicCameraComponent::EndModeLocked()
{
	DoActionCamera.Unbind();

	if (IsValid(CurrentTarget))
	{
		Cast<ITargetable>(CurrentTarget)->Execute_ToggleLock(CurrentTarget, false);
	}

	CurrentTarget = nullptr;
	TargetLocked = false;
}

void UDynamicCameraComponent::InvertYAxis(bool IsInverted)
{
	YAxisDirection = IsInverted ? -1 : 1;
}

void UDynamicCameraComponent::NavigateTargets(float AxisValue)
{
	// Check threshold.
	if (FMath::Abs(AxisValue) < NavigateThreshold)
	{
		prevNavIncrementSign = 0;
		return;
	}

	// Avoid infinite scroll.
	int IncrementSign = AxisValue > 0 ? 1 : -1;
	if (IncrementSign == prevNavIncrementSign)
		return;

	if (ObjectsInRange.Num() <= 0)
	{
		SetModeFree(CameraStates::LOCKED);
		return;
	}

	struct FSignedAngleSort
	{
		FVector ComponentLocation;
		FVector TargetDir;

		bool operator()(const AActor& lhs, const AActor& rhs) const
		{
			// Sort by singed angle.

			FVector LhsDir = (lhs.GetActorLocation() - ComponentLocation).GetSafeNormal();
			float LhsAngle = FMath::Acos(FVector::DotProduct(TargetDir, LhsDir)) * FMath::Sign(FVector::DotProduct(FVector::CrossProduct(LhsDir, TargetDir), FVector::UpVector));

			FVector RhsDir = (rhs.GetActorLocation() - ComponentLocation).GetSafeNormal();
			float RhsAngle = FMath::Acos(FVector::DotProduct(TargetDir, RhsDir)) * FMath::Sign(FVector::DotProduct(FVector::CrossProduct(RhsDir, TargetDir), FVector::UpVector));

			return LhsAngle > RhsAngle;
		}
	};

	ObjectsInRange.Sort(FSignedAngleSort{ GetComponentLocation(), (CurrentTarget->GetActorLocation() - GetComponentLocation()).GetSafeNormal() });

	//TODO Maxence: instead of setting camera to free mode set CurrTargetIndex to 0 can be harzardous so do not do it for BETA build
	int CurrTargetIndex = ObjectsInRange.Find(CurrentTarget);
	if (CurrTargetIndex < 0)
	{
		SetModeFree(CameraStates::LOCKED);
		return;
	}

	// Increment and clamp.
	FHitResult HitInfos;

	int PrevTargetIndex = CurrTargetIndex;
	int TargetIndex = CurrTargetIndex + IncrementSign;
	FVector PrevTargetDir = (ObjectsInRange[PrevTargetIndex]->GetActorLocation() - GetComponentLocation()).GetSafeNormal();

	for (; TargetIndex != CurrTargetIndex; TargetIndex += IncrementSign)
	{
		// Clamp.
		if (TargetIndex < 0)
			TargetIndex = ObjectsInRange.Num() - 1;
		else if (TargetIndex >= ObjectsInRange.Num())
			TargetIndex = 0;

		if (ObjectsInRange[TargetIndex] == CurrentTarget)
			break;


		if (!bCyclicNavigation)
		{
			// Min or max reached.
			FVector TargetDir = (ObjectsInRange[TargetIndex]->GetActorLocation() - GetComponentLocation()).GetSafeNormal();

			if (FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(PrevTargetDir, TargetDir))) >= MaxAngleNavigation)
				return;

			PrevTargetDir = TargetDir;
		}

		// Instant select valid target.
		if (!bNavigateOnlyVisible)
			break;

		// Check visiiblity: break on first no hit.
		if (!GetWorld()->LineTraceSingleByChannel(HitInfos, Camera->GetComponentLocation() + NavigationRaycastOffset, ObjectsInRange[TargetIndex]->GetActorLocation(), ECollisionChannel::ECC_Visibility)
			|| HitInfos.Actor.Get() == ObjectsInRange[TargetIndex])
		{
			break;
		}
	}

	if (TargetIndex == CurrTargetIndex)
		return;

	CurrTargetIndex = TargetIndex;
	prevNavIncrementSign = IncrementSign;

	SetCurrentTarget(ObjectsInRange[CurrTargetIndex]);
}

void UDynamicCameraComponent::TargetClosestAngle()
{
	FHitResult HitInfos;
	int bestId = -1;

	float bestDistance = autoLockedDistance;
	for (int targetId = 0; targetId < ObjectsInRange.Num(); ++targetId)
	{
		if (ObjectsInRange[targetId] == CurrentTarget)
			continue;

		// Instant select valid target.
		if (bNavigateOnlyVisible)
		{
			// Check visibility
			if (GetWorld()->LineTraceSingleByChannel(HitInfos, Camera->GetComponentLocation() + NavigationRaycastOffset, ObjectsInRange[targetId]->GetActorLocation(), ECollisionChannel::ECC_Visibility)
				&& HitInfos.Actor.Get() != ObjectsInRange[targetId])
			{
				continue;
			}
		}

		float dist = GetOwner()->GetDistanceTo(ObjectsInRange[targetId]);
		if (dist < bestDistance)
		{
			bestDistance = dist;
			bestId = targetId;
		}
	}

	// No visible target: unlock.
	if (bestId >= 0 && bestId < ObjectsInRange.Num())
		SetCurrentTarget(ObjectsInRange[bestId]);
	else
		SetCurrentTarget(nullptr);
}

void UDynamicCameraComponent::SetCurrentTarget(AActor* NewTarget)
{
	// Unlock previous target.
	if (IsValid(CurrentTarget))
	{
		Cast<ITargetable>(CurrentTarget)->Execute_ToggleLock(CurrentTarget, false);
	}
	else
		OnCameraLock.Broadcast();

	// Lock new Target.
	CurrentTarget = NewTarget;

	if (IsValid(CurrentTarget))
	{
		Cast<ITargetable>(CurrentTarget)->Execute_ToggleLock(CurrentTarget, true);
	}
	else
		SetModeFree(CameraStates::LOCKED);

	OnCameraChangeTarget.Broadcast(CurrentTarget);
}


void UDynamicCameraComponent::LookAt(FRotator& ReturnRotation)
{
	float deltaSeconds = GetWorld()->GetDeltaSeconds();
	FRotator controllerRotation = GetOwner()->GetInstigatorController()->GetControlRotation();
	FRotator temp = UKismetMathLibrary::RInterpTo(controllerRotation,
		UKismetMathLibrary::FindLookAtRotation(Camera->GetComponentLocation(), CurrentTarget->GetActorLocation()), deltaSeconds, RotationInterpSpeed);
	temp.Roll = controllerRotation.Roll;
	if (!TargetLocked)
	{
		temp.Pitch = FMath::Clamp(controllerRotation.Pitch, MinPitchAngle, MaxPitchAngle);
	}
	else
	{
		temp.Pitch = FMath::Clamp(temp.Pitch, MinPitchAngleWhenLocked, MaxPitchAngleWhenLocked);
	}
	ReturnRotation = temp;
}

void UDynamicCameraComponent::MoveCamera(FVector NewPos, FVector& LerpedPos, float& Length)
{
	float deltaSeconds = GetWorld()->GetDeltaSeconds();
	FVector temp = NewPos;
	temp.X = 0;

	Length = UKismetMathLibrary::FInterpTo(TargetArmLength, DistanceCameraWhenLocked, deltaSeconds, RotationInterpSpeed);
	LerpedPos = UKismetMathLibrary::VInterpTo(SocketOffset, temp, deltaSeconds, RotationInterpSpeed);
}

bool UDynamicCameraComponent::IsInArray(AActor* OtherObject)
{
	return ObjectsInRange.Contains(OtherObject);
}

AActor* UDynamicCameraComponent::GetCurrentTarget() const
{
	return CurrentTarget;
}

void UDynamicCameraComponent::SetCameraReseting(bool _Value)
{
	IsCameraReseting = _Value;
	ResetCurrentTime = _Value ? TimeBeforeReset : 0;
}

#pragma endregion

