// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "Maxence_SandboxCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Characters/Components/DynamicCameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"

//////////////////////////////////////////////////////////////////////////
// AMaxence_SandboxCharacter

AMaxence_SandboxCharacter::AMaxence_SandboxCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<UDynamicCameraComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

}

//////////////////////////////////////////////////////////////////////////
// Input

void AMaxence_SandboxCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMaxence_SandboxCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMaxence_SandboxCharacter::MoveRight);

	// handle camera movement
	InputComponent->BindAxis("RightThumbstickXAxis", this, &AMaxence_SandboxCharacter::CameraMoveRight);
	InputComponent->BindAxis("RightThumbstickXAxisLocked", this, &AMaxence_SandboxCharacter::CameraMoveRightLocked);
	InputComponent->BindAxis("RightThumbstickYAxis", this, &AMaxence_SandboxCharacter::CameraMoveForward);

	InputComponent->BindAction("RightThumbstickButton", EInputEvent::IE_Pressed, this, &AMaxence_SandboxCharacter::PressedTargettingButton);

}


void AMaxence_SandboxCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AMaxence_SandboxCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AMaxence_SandboxCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AMaxence_SandboxCharacter::MoveRight(float Value)
{
	if ( (Controller != NULL) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void AMaxence_SandboxCharacter::CameraMoveRight_Implementation(float _AxisInput)
{
	if (DisableInputs)
		return;

	if (CameraBoom->TargetLocked)
	{
		return;
	}

	if (FMath::Abs(_AxisInput) > 0.1f)
		CameraBoom->PreventResetCamera(true);
	AddControllerYawInput(_AxisInput * BaseTurnRate * (IsXAxisInverted ? -1 : 1) * GetWorld()->DeltaTimeSeconds);

}

void AMaxence_SandboxCharacter::CameraMoveRightLocked_Implementation(float _AxisInput)
{
	if (DisableInputs)
		return;

	if (!CameraBoom->TargetLocked)
	{
		return;
	}

	CameraBoom->NavigateTargets(_AxisInput);

}


void AMaxence_SandboxCharacter::CameraMoveForward_Implementation(float _AxisInput)
{
	if (DisableInputs)
		return;

	if (CameraBoom)
	{
		if (FMath::Abs(_AxisInput) > 0.1f)
			CameraBoom->PreventResetCamera(true);
		AddControllerPitchInput(_AxisInput * BaseLookUpRate * CameraBoom->YAxisDirection * GetWorld()->DeltaTimeSeconds);
	}
}

void AMaxence_SandboxCharacter::PressedTargettingButton_Implementation()
{
	if (CameraBoom->TargetLocked)
	{
		CameraBoom->SetModeFree(CameraStates::LOCKED);
	}
	else
	{
		CameraBoom->SetModeLocked(CameraStates::FREE);
	}

}