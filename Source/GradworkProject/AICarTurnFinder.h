// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AICarController.h"
#include "AICarTurnFinder.generated.h"


class UChaosVehicleMovementComponent;
class UChaosWheeledVehicleMovementComponent;

/**
 * Drives the car at a constant speed, 
 * tries to make the smallest turn while not slipping
 */
UCLASS()
class GRADWORKPROJECT_API AAICarTurnFinder : public AAICarController
{
	GENERATED_BODY()


protected:
	/** Desired speed **/
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float TargetSpeed = 50.0f;

	/** Max steering angle (0 - 1.0f) **/
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MaxSteeringAngle = 1.0f;

	/** Invert Steering Direction (Go left instead of right) **/
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bInvertSteeringDirection = false;


protected:
	virtual void BeginPlay() override;
public:
	virtual void Tick(float Delta) override;

private:
	UChaosVehicleMovementComponent* VehicleMovement = nullptr;
	UChaosWheeledVehicleMovementComponent * ChaosVehicleMovement = nullptr;
	
	float startDelay = 5.0f; // Delay before starting the script
	bool hasStarted = false;

	const float validationTime = 5.0f; // Time to validate a steering angle
	float validationTimer = 0.0f; // Timer for validation
	
	const float stepTime = 0.5f; // Time to wait between steering angle changes
	float stepTimer = 0.0f; // Timer for steering angle changes

	bool isValidated = false; // Is the current steering angle validated
	float bestSteeringInput = 0.0f; // Best steering angle so far
	float currentSteeringInput = 1.0f;
	float stepSizeSteeringInput = 0.01f;


	// Speed control
	float currentThrottleInput = 0.0f;
	const float maxThrottleChangePerSecond = 0.1f;


private:
	bool KeepTargetSpeed(float Delta);

protected:
	UFUNCTION(BlueprintCallable)
	bool GetIsValidated() const { return isValidated; }

	UFUNCTION(BlueprintCallable)
	float GetCurrentSteeringAngle() const;

};
