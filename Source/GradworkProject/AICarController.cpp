// Fill out your copyright notice in the Description page of Project Settings.


#include "AICarController.h"
#include "GradworkProjectPawn.h"
#include "EnhancedInputSubsystems.h"
#include "ChaosWheeledVehicleMovementComponent.h"

void AAICarController::BeginPlay()
{
	Super::BeginPlay();
}

void AAICarController::Tick(float Delta)
{
	Super::Tick(Delta);
}

void AAICarController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// get a pointer to the controlled pawn
	VehiclePawn = CastChecked<AGradworkProjectPawn>(InPawn);
	

	VehiclePawn->GetVehicleMovementComponent()->SetUseAutomaticGears(true);
	VehiclePawn->GetVehicleMovementComponent()->SetTargetGear(1, true);

	// Fire start sequence after delay
	FTimerHandle UnusedHandle;
	GetWorldTimerManager().SetTimer(UnusedHandle, this, &AAICarController::StartSequence, StartDelay, false);
}