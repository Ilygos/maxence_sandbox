// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <GameFramework/SpringArmComponent.h>
#include "DynamicCameraComponent.generated.h"

UENUM()
enum CameraStates
{
	FREE,
	LOCKED,
	CVOID
};
class UCameraComponent;
class UTargetable;
#define MIN_LEFT_ANGLE 181
#define MAX_LEFT_ANGLE 359
#define MIN_RIGHT_ANGLE 1
#define MAX_RIGHT_ANGLE 179

DECLARE_DELEGATE(Action);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FChangingState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FChangingTarget, AActor*, NewTarget);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSwitchingTargetDelegate, AActor*, EnemyElement, AActor*, CurrentTarget);

UCLASS(BlueprintType, Blueprintable)
class UDynamicCameraComponent : public USpringArmComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	UDynamicCameraComponent(const FObjectInitializer& ObjectInitializer);

	/**	Character's camera.*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
		UCameraComponent* Camera = nullptr;
	/**	Character camera's collider.*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
		class	USphereComponent* CameraRangeSphere = nullptr;

	//Boom anchor allows the camera to be slightly skewed
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
		UStaticMeshComponent* BoomAnchor;

	//Socket offset position when camera free
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
		FVector PositionOffsetFree;
protected:
#pragma region CAMERA
	/** VARIABLES */

	float ResetCurrentTime;
	/// The current target
	AActor* CurrentTarget;

	const TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypesLock{ EObjectTypeQuery::ObjectTypeQuery1, EObjectTypeQuery::ObjectTypeQuery2, EObjectTypeQuery::ObjectTypeQuery3 };

	/** METHODS */
	/// DoAction camera. Basically acting like the update. 
/// We bind methods to this delegate according to the current state of the camera
	Action DoActionCamera;

	void HandlingFinishedState(CameraStates PrevState);
	UFUNCTION()
		/// Called when object enters camera range.
		void OnObjectEntersRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
		/// Called when object leaves camera range.
		void OnObjectLeavesRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	/// Does the action camera locked.
	UFUNCTION()
		void DoActionLocked();

	/// Does the action camera free.
	UFUNCTION()
		void DoActionFree();


#pragma endregion

	void BeginPlay() override;
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:

#pragma region CAMERA
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[STARK]|Kojima Camera")
		float YAxisDirection;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[STARK]|Kojima Camera")
		float BaseTurnRate;

	/** Look at camera bone. Advanced settings. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[STARK]|Kojima Camera")
		FName LookAtCameraBone = "LookAt_J";

	/// The objects in range
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "[STARK]|Kojima Camera")
		TArray<AActor*> ObjectsInRange;

	/// The minimum pitch when target unlocked
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[STARK]|Kojima Camera|Unlocked Mode")
		float MinPitchAngle;

	/// The maximum pitch when target unlocked
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[STARK]|Kojima Camera|Unlocked Mode")
		float MaxPitchAngle;


	/// The minimum pitch when target locked
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[STARK]|Kojima Camera|Locked Mode")
		float MinPitchAngleWhenLocked = -25;

	/// The maximum pitch when target locked
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[STARK]|Kojima Camera|Locked Mode")
		float MaxPitchAngleWhenLocked = 25;


	/// The distance camera when unlocked
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[STARK]|Kojima Camera|Unlocked Mode")
		float DistanceCameraWhenUnlocked;

	/// The distance camera when locked
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[STARK]|Kojima Camera|Locked Mode")
		float DistanceCameraWhenLocked;

	/// The minimum range to select
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[STARK]|Kojima Camera|Locked Mode")
		float MinimumRangeToSelect;

	/// The closest target distance
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "[STARK]|Kojima Camera|Locked Mode")
		float ClosestTargetDistance;

	/// The rotation interp speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[STARK]|Kojima Camera")
		float RotationInterpSpeed;

	/// The focus interp speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[STARK]|Kojima Camera|Locked Mode")
		float FocusInterpSpeed;

	/// The base look up rate
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[STARK]|Kojima Camera")
		float BaseLookUpRate;

	/// The target when camera locked
	UPROPERTY(BlueprintReadOnly, Category = "[STARK]|Kojima Camera|Locked Mode")
		bool TargetLocked;

	/// previous navigation incrementation sign.
	float prevNavIncrementSign;

	/// Input threshold to navigate.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[STARK]|Kojima Camera|Locked Mode")
		float NavigateThreshold;

	/// Maximum angle before stopping navigation while bCyclicNavigation == false.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[STARK]|Kojima Camera", meta = (EditCondition = "!bCyclicNavigation"))
		float MaxAngleNavigation;

	/// Should make the navigation cyclic.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[STARK]|Kojima Camera|Locked Mode")
		bool bCyclicNavigation;

	/// Should navigate through only visible Targets.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[STARK]|Kojima Camera|Locked Mode")
		bool bNavigateOnlyVisible;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[STARK]|Kojima Camera|Locked Mode")
		float autoLockedDistance = 1000.f;

	/// Offset to raycast from the player (head offset).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[STARK]|Kojima Camera|Locked Mode")
		FVector NavigationRaycastOffset;

	/// The anchor when camera locked
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[STARK]|Kojima Camera|Locked Mode")
		FVector AnchorLocked;

	/// Anchor Offset so we don't just lock on the player directly ??????
	UPROPERTY(BlueprintReadOnly, Category = "[STARK]|Kojima Camera|Locked Mode")
		AActor* ChosenEnemy;

	////Reset Camera /////
	bool IsCameraReseting;

	/// Rate speed of camera reset
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[STARK]|Kojima Camera|Reset")
		float ResetCameraRate;

	/// Time before the reset boolean is setting back to true
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[STARK]|Kojima Camera|Reset")
		float TimeBeforeReset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[STARK]|Kojima Camera|Reset")
		bool ForceReset;

	/// Angle Value in Radian to prevent reseting while facing camera
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[STARK]|Kojima Camera|Reset")
		float FacingAngleNotReseting;
	/** METHODS */

	/// <summary>
	/// Navigates between targets following an axis value.
	/// </summary>
	/// <param name="AxisValue">The axis value.</param>
	UFUNCTION(BlueprintCallable, Category = "[STARK]|Kojima Camera")
		void NavigateTargets(float AxisValue);

	/** Callback when camera start lock mode. */
	UPROPERTY(BlueprintAssignable, Category = "Lock")
		FChangingState OnCameraLock;


	/** Callback when camera stop lock mode. */
	UPROPERTY(BlueprintAssignable, Category = "Lock")
		FChangingState OnCameraUnlock;

	// Event to switch target Between left and right
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Lock")
		FSwitchingTargetDelegate OnSwapRight;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Lock")
		FSwitchingTargetDelegate OnSwapLeft;

	/** Callback when camera changes target. */
	UPROPERTY(BlueprintAssignable, Category = "Lock")
		FChangingTarget OnCameraChangeTarget;

	UFUNCTION(BlueprintCallable, meta = (Category, OverrideNativeName = "SetModeFree"))
		virtual void SetModeFree(CameraStates PrevState);

	/// Sets the mode locked camera.
	UFUNCTION(BlueprintCallable, meta = (Category, OverrideNativeName = "SetModeLocked"))
		virtual void SetModeLocked(CameraStates PrevState);

	/// Ends the mode free camera.
	UFUNCTION(BlueprintCallable, meta = (Category, OverrideNativeName = "EndModeFree"))
		virtual void EndModeFree();

	/// Ends the mode locked camera.
	UFUNCTION(BlueprintCallable, meta = (Category, OverrideNativeName = "EndModeLocked"))
		virtual void EndModeLocked();

	/// Prevent reset camera
	UFUNCTION(BlueprintCallable, meta = (Category, OverrideNativeName = "PreventResetCamera"))
		void PreventResetCamera(bool _HardReset);

	/// Invert y axis
	/// <param name="IsInverted">Whether the axis is inverted or not</param>
	UFUNCTION(BlueprintCallable, meta = (Category, OverrideNativeName = "InvertYAxis"))
		virtual void InvertYAxis(bool IsInverted);

	/// Looks at.
	/// <param name="ReturnRotation">The return rotation.</param>
	UFUNCTION(BlueprintCallable, meta = (Category, OverrideNativeName = "LookAt"))
		virtual void LookAt(/*out*/ FRotator& ReturnRotation);

	/// Moves the camera.
	/// <param name="NewPos">The new position.</param>
	/// <param name="LerpedPos">The lerped position.</param>
	/// <param name="Length">The length.</param>
	UFUNCTION(BlueprintCallable, meta = (Category, OverrideNativeName = "MoveCamera"))
		virtual void MoveCamera(FVector NewPos, /*out*/ FVector& LerpedPos, /*out*/ float& Length);

	/// Determines whether is in array or not.
	/// <param name="OtherObject">The other object.</param>
	/// <returns>
	///   <c>true</c> if is in array; otherwise, <c>false</c>.
	/// </returns>
	UFUNCTION(BlueprintCallable, meta = (Category, OverrideNativeName = "IsInArray"))
		virtual bool IsInArray(AActor* OtherObject);

	/** Return the current target. */
	UFUNCTION(BlueprintCallable)
		AActor* GetCurrentTarget() const;

	UFUNCTION(BlueprintCallable, Category = "Camera")
		void SetCurrentTarget(AActor* NewTarget);

	/*UFUNCTION(Category = "Camera")
		void OnTargetDie(ITargetable* _Target);*/

	UFUNCTION(BlueprintCallable, Category = "Camera")
		void SetCameraReseting(bool _Value);

	/** Set new target to closest angle. */
	void TargetClosestAngle();


	void ResetCamera(float DeltaSeconds);
#pragma endregion

	// Called every frame
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction) override;

};
