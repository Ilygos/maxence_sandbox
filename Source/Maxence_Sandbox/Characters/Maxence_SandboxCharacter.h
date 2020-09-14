// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Maxence_SandboxCharacter.generated.h"

UCLASS(config=Game)
class AMaxence_SandboxCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UDynamicCameraComponent* CameraBoom;

public:
	AMaxence_SandboxCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	bool DisableInputs;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	bool IsXAxisInverted;

protected:

	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class UDynamicCameraComponent* GetCameraBoom() const { return CameraBoom; }

	/** Function bound on camera move right/left input (ie: RightThumbstickXAxis). */
	UFUNCTION(BlueprintNativeEvent, Category = "CameraMovement")
		void CameraMoveRight(float _AxisInput);

	UFUNCTION(BlueprintNativeEvent, Category = "CameraMovement")
		void CameraMoveRightLocked(float _AxisInput);

	/** Function bound on camera move forward/backward input (ie: RightThumbstickYAxis). */
	UFUNCTION(BlueprintNativeEvent, Category = "CameraMovement")
		void CameraMoveForward(float _AxisInput);

	/** Function bound on Right thumbstick input (ie: R3Button). */
	UFUNCTION(BlueprintNativeEvent, Category = "Misc")
	void PressedTargettingButton();
};

