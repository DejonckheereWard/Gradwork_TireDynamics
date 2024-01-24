// Fill out your copyright notice in the Description page of Project Settings.


#include "AICarTurnFinder.h"
#include "AICarController.h"
#include "GradworkProjectPawn.h"
#include "ChaosWheeledVehicleMovementComponent.h"

void AAICarTurnFinder::BeginPlay()
{
	Super::BeginPlay();

	VehicleMovement = VehiclePawn->GetVehicleMovement();
	ChaosVehicleMovement = CastChecked<UChaosWheeledVehicleMovementComponent>(VehicleMovement);


}

void AAICarTurnFinder::Tick(float Delta)
{
	Super::Tick(Delta);

	if (!hasStarted) {
		startDelay -= Delta;
		if (startDelay <= 0.0f) {
			hasStarted = true;
		}
		return;
	}

	bool isAtTargetSpeed = KeepTargetSpeed(Delta);

	// Check for slipping & skidding
	bool isSlipping = false;
	bool isSkidding = false;
	for (int32 i = 0; i < ChaosVehicleMovement->Wheels.Num(); i++)
	{
		UChaosVehicleWheel* Wheel = ChaosVehicleMovement->Wheels[i];
		FWheelStatus WheelState = ChaosVehicleMovement->GetWheelState(i);

		if(WheelState.bIsSkidding)
		{
			isSkidding = true;
		}
		if(WheelState.bIsSlipping)
		{
			isSlipping = true;
		}
	}

	// Only validate if we are at target speed
	if(!isAtTargetSpeed)
	{
		validationTimer = 0.0f;
		stepTimer = 0.0f;
		return;
	}


	if (isSlipping || isSkidding) {
		// Reset validation if we are slipping or skidding
		validationTimer = 0.0f;

		// Lower steering angle, but we only do this every at max every 0.5 seconds
		// This is to prevent the car from lowering the steering angle too fast & having enough time to stabilize
		if (stepTimer > stepTime) {
			currentSteeringInput -= stepSizeSteeringInput;  // Lower steering angle by one step
			currentSteeringInput = FMath::Clamp(currentSteeringInput, 0.0f, 1.0f);  // Clamp steering angle between 0 and 1
			stepTimer = 0.0f;
		}
	}
	else {
		if (validationTimer > validationTime) {
			isValidated = true;
			bestSteeringInput = currentSteeringInput;
		}
	}

	// Update steering input	
	if(bInvertSteeringDirection) {
		VehicleMovement->SetSteeringInput(-currentSteeringInput);
	}
	else
	{
		VehicleMovement->SetSteeringInput(currentSteeringInput);
	}

	validationTimer += Delta;
	stepTimer += Delta;
}

bool AAICarTurnFinder::KeepTargetSpeed(float Delta)
{
	// Keep target speed
	// * 0.036 because speed is in cm/s and we want km/h (1 cm/s = 0.036 km/h)
	const float currentSpeed = Chaos::CmSToKmH(VehicleMovement->GetForwardSpeed());
	
	// Smoothly adjust throttle input to get / keep target speed
	// Note, throttle is a value 0-1, and speed is in km/h
	const float speedError = TargetSpeed - currentSpeed;
	const float adjustmentFactor = 0.3f;
	float adjustment = speedError * adjustmentFactor;
	adjustment = FMath::Clamp(adjustment, -maxThrottleChangePerSecond * Delta, maxThrottleChangePerSecond * Delta);
	currentThrottleInput = FMath::Clamp(currentThrottleInput + adjustment, 0.0f, 1.0f);
	VehicleMovement->SetThrottleInput(currentThrottleInput);
	

	if (speedError < 2.0f)
	{
		return true;
	}
	else
	{
		return false;
	}
}

float AAICarTurnFinder::GetCurrentSteeringAngle() const
{
	for (UChaosVehicleWheel* Wheel : ChaosVehicleMovement->Wheels)
	{
		//UChaosVehicleWheel* Wheel = ChaosVehicleMovement->Wheels[i];
		//FWheelStatus WheelState = ChaosVehicleMovement->GetWheelState(i);
		if (Wheel->AxleType == EAxleType::Front)
		{
			return FMath::Abs(Wheel->GetSteerAngle());
		}
	}

	return -1.0f;
}
