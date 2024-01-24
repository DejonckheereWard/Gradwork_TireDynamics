// Copyright Epic Games, Inc. All Rights Reserved.

#include "GradworkProjectPawn.h"
#include "GradworkProjectWheelFront.h"
#include "GradworkProjectWheelRear.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/AudioComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"

#define LOCTEXT_NAMESPACE "VehiclePawn"

DEFINE_LOG_CATEGORY(LogTemplateVehicle);

AGradworkProjectPawn::AGradworkProjectPawn()
{
	// Construct the front camera boom
	FrontSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Front Spring Arm"));
	FrontSpringArm->SetupAttachment(GetMesh());
	FrontSpringArm->TargetArmLength = 0.0f;
	FrontSpringArm->bDoCollisionTest = false;
	FrontSpringArm->bEnableCameraRotationLag = true;
	FrontSpringArm->CameraRotationLagSpeed = 15.0f;
	FrontSpringArm->SetRelativeLocation(FVector(30.0f, 0.0f, 120.0f));

	FrontCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Front Camera"));
	FrontCamera->SetupAttachment(FrontSpringArm);
	FrontCamera->bAutoActivate = false;

	// Construct the back camera boom
	BackSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Back Spring Arm"));
	BackSpringArm->SetupAttachment(GetMesh());
	BackSpringArm->TargetArmLength = 650.0f;
	BackSpringArm->SocketOffset.Z = 150.0f;
	BackSpringArm->bDoCollisionTest = false;
	BackSpringArm->bInheritPitch = false;
	BackSpringArm->bInheritRoll = false;
	BackSpringArm->bEnableCameraRotationLag = true;
	BackSpringArm->CameraRotationLagSpeed = 2.0f;
	BackSpringArm->CameraLagMaxDistance = 50.0f;

	BackCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Back Camera"));
	BackCamera->SetupAttachment(BackSpringArm);

	// Create audio component
	EngineSoundComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("EngineSoundComponent"));
	EngineSoundComponent->SetupAttachment(GetMesh());

	// Configure the car mesh
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionProfileName(FName("Vehicle"));

	// get the Chaos Wheeled movement component
	ChaosVehicleMovement = CastChecked<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement());
}

void AGradworkProjectPawn::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Steering 
		EnhancedInputComponent->BindAction(SteeringAction, ETriggerEvent::Triggered, this, &AGradworkProjectPawn::Steering);
		EnhancedInputComponent->BindAction(SteeringAction, ETriggerEvent::Completed, this, &AGradworkProjectPawn::Steering);

		// Throttle 
		EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Triggered, this, &AGradworkProjectPawn::Throttle);
		EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Completed, this, &AGradworkProjectPawn::Throttle);

		// Break 
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Triggered, this, &AGradworkProjectPawn::Brake);
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Started, this, &AGradworkProjectPawn::StartBrake);
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Completed, this, &AGradworkProjectPawn::StopBrake);

		// Handbrake 
		EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Started, this, &AGradworkProjectPawn::StartHandbrake);
		EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Completed, this, &AGradworkProjectPawn::StopHandbrake);

		// Look around 
		EnhancedInputComponent->BindAction(LookAroundAction, ETriggerEvent::Triggered, this, &AGradworkProjectPawn::LookAround);

		// Toggle camera 
		EnhancedInputComponent->BindAction(ToggleCameraAction, ETriggerEvent::Triggered, this, &AGradworkProjectPawn::ToggleCamera);

		// Reset the vehicle 
		EnhancedInputComponent->BindAction(ResetVehicleAction, ETriggerEvent::Triggered, this, &AGradworkProjectPawn::ResetVehicle);

		// Shift up
		EnhancedInputComponent->BindAction(ShiftUpAction, ETriggerEvent::Triggered, this, &AGradworkProjectPawn::ShiftUp);

		// Shift down
		EnhancedInputComponent->BindAction(ShiftDownAction, ETriggerEvent::Triggered, this, &AGradworkProjectPawn::ShiftDown);
	}
	else
	{
		UE_LOG(LogTemplateVehicle, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AGradworkProjectPawn::BeginPlay()
{
	Super::BeginPlay();

	// Get the number of wheels and resize the skid particle array
	int32 NumWheels = ChaosVehicleMovement->Wheels.Num();
	SkidParticleComponents.SetNum(NumWheels);

	// Call the checkdrift event every 0.05 seconds or 20 times a second
	FTimerHandle UnusedHandle;
	GetWorldTimerManager().SetTimer(UnusedHandle, this, &AGradworkProjectPawn::OnHandleDrift, 0.05f, true);
}

void AGradworkProjectPawn::Tick(float Delta)
{
	Super::Tick(Delta);

	// add some angular damping if the vehicle is in midair
	bool bMovingOnGround = ChaosVehicleMovement->IsMovingOnGround();
	GetMesh()->SetAngularDamping(bMovingOnGround ? 0.0f : 3.0f);

	// realign the camera yaw to face front
	float CameraYaw = BackSpringArm->GetRelativeRotation().Yaw;
	CameraYaw = FMath::FInterpTo(CameraYaw, 0.0f, Delta, 1.0f);

	BackSpringArm->SetRelativeRotation(FRotator(0.0f, CameraYaw, 0.0f));

	// update the engine sound

	const float CurrentRpm = ChaosVehicleMovement->GetEngineRotationSpeed();
	const float minRpm = ChaosVehicleMovement->EngineSetup.EngineIdleRPM;
	const float maxRpm = ChaosVehicleMovement->EngineSetup.MaxRPM;

	float RpmRange = FMath::GetMappedRangeValueClamped(FVector2D(minRpm, maxRpm), FVector2D(0.0f, 1.0f), CurrentRpm);
	EngineSoundComponent->SetFloatParameter(SoundInputName, RpmRange);
}

void AGradworkProjectPawn::Steering(const FInputActionValue& Value)
{
	// get the input magnitude for steering
	float SteeringValue = Value.Get<float>();

	// add the input
	ChaosVehicleMovement->SetSteeringInput(SteeringValue);
}

void AGradworkProjectPawn::Throttle(const FInputActionValue& Value)
{
	// get the input magnitude for the throttle
	float ThrottleValue = Value.Get<float>();

	// add the input
	ChaosVehicleMovement->SetThrottleInput(ThrottleValue);
}

void AGradworkProjectPawn::Brake(const FInputActionValue& Value)
{
	// get the input magnitude for the brakes
	float BreakValue = Value.Get<float>();

	// add the input
	ChaosVehicleMovement->SetBrakeInput(BreakValue);
}

void AGradworkProjectPawn::StartBrake(const FInputActionValue& Value)
{
	// call the Blueprint hook for the break lights
	BrakeLights(true);
}

void AGradworkProjectPawn::StopBrake(const FInputActionValue& Value)
{
	// call the Blueprint hook for the break lights
	BrakeLights(false);

	// reset brake input to zero
	ChaosVehicleMovement->SetBrakeInput(0.0f);
}

void AGradworkProjectPawn::StartHandbrake(const FInputActionValue& Value)
{
	// add the input
	ChaosVehicleMovement->SetHandbrakeInput(true);

	// call the Blueprint hook for the break lights
	BrakeLights(true);
}

void AGradworkProjectPawn::StopHandbrake(const FInputActionValue& Value)
{
	// add the input
	ChaosVehicleMovement->SetHandbrakeInput(false);

	// call the Blueprint hook for the break lights
	BrakeLights(false);
}

void AGradworkProjectPawn::LookAround(const FInputActionValue& Value)
{
	// get the flat angle value for the input 
	float LookValue = Value.Get<float>();

	// add the input
	BackSpringArm->AddLocalRotation(FRotator(0.0f, LookValue, 0.0f));
}

void AGradworkProjectPawn::ToggleCamera(const FInputActionValue& Value)
{
	// toggle the active camera flag
	bFrontCameraActive = !bFrontCameraActive;

	FrontCamera->SetActive(bFrontCameraActive);
	BackCamera->SetActive(!bFrontCameraActive);
}

void AGradworkProjectPawn::ResetVehicle(const FInputActionValue& Value)
{
	// reset to a location slightly above our current one
	FVector ResetLocation = GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);

	// reset to our yaw. Ignore pitch and roll
	FRotator ResetRotation = GetActorRotation();
	ResetRotation.Pitch = 0.0f;
	ResetRotation.Roll = 0.0f;
	
	// teleport the actor to the reset spot and reset physics
	SetActorTransform(FTransform(ResetRotation, ResetLocation, FVector::OneVector), false, nullptr, ETeleportType::TeleportPhysics);

	GetMesh()->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	GetMesh()->SetPhysicsLinearVelocity(FVector::ZeroVector);

	UE_LOG(LogTemplateVehicle, Error, TEXT("Reset Vehicle"));
}

void AGradworkProjectPawn::ShiftUp(const FInputActionValue& Value)
{
	int32 TargetGear = ChaosVehicleMovement->GetCurrentGear() + 1;
	ChaosVehicleMovement->SetTargetGear(TargetGear, false);
}

void AGradworkProjectPawn::ShiftDown(const FInputActionValue& Value)
{
	int32 TargetGear = ChaosVehicleMovement->GetCurrentGear() - 1;
	ChaosVehicleMovement->SetTargetGear(TargetGear, false);
}

void AGradworkProjectPawn::OnHandleDrift_Implementation()
{
	HandleDriftEffects();
}

void AGradworkProjectPawn::HandleDriftEffects()
{
	if (!bSkidMarks)
	{
		return;
	}

	// Loop over all the wheels
	EVehicleDifferential DifferentialType = GetChaosVehicleMovement()->DifferentialSetup.DifferentialType;

	for (int32 i = 0; i < ChaosVehicleMovement->Wheels.Num(); i++)
	{
		// Get the wheel
		UChaosVehicleWheel* Wheel = ChaosVehicleMovement->Wheels[i];
		FWheelStatus WheelState = ChaosVehicleMovement->GetWheelState(i);

		// Early out if the wheel is not powered
		if(DifferentialType == EVehicleDifferential::RearWheelDrive && Wheel->AxleType != EAxleType::Rear) {
			continue;
		}
		if (DifferentialType == EVehicleDifferential::FrontWheelDrive && Wheel->AxleType != EAxleType::Front) {
			continue;
		}
		
		// If wheel isnt in the air and is slipping or skidding
		if (!Wheel->IsInAir() &&
			(WheelState.bIsSlipping || WheelState.bIsSkidding))
		{
			// Do the skid effects
			//UE_LOG(LogTemplateVehicle, Log, TEXT("Skidding"));

			// Only spawn the particle if it doesn't exist already
			if (!IsValid(SkidParticleComponents[i]))
			{
				// Get the contact location
				FVector ContactLocation = WheelState.HitLocation;

				// Get the mesh location
				FVector MeshLocation = GetMesh()->GetComponentLocation();

				// Get the vector between the mesh and the contact location
				FVector RelativeOffset = ContactLocation - MeshLocation;

				// Spawn the skid particle attached
				UNiagaraComponent* SpawnedParticle = UNiagaraFunctionLibrary::SpawnSystemAttached(SkidParticle, GetMesh(), FName(""), RelativeOffset, WheelState.SkidNormal.Rotation(), EAttachLocation::KeepRelativeOffset, false);
				// Get the sign of the skid magnitude
				const float SkidMagnitude = FMath::Sign(WheelState.SkidMagnitude);
				SpawnedParticle->SetNiagaraVariableFloat("SmokeSpawnRate", SkidMagnitude);

				SkidParticleComponents[i] = SpawnedParticle;
			}
			else {
				// Update smoke spawn rate
				const float SkidMagnitude = FMath::Sign(WheelState.SkidMagnitude);
				SkidParticleComponents[i]->SetNiagaraVariableFloat("SmokeSpawnRate", SkidMagnitude);

				// Update the opacity of the skid marks
				const float SlipValue = FMath::GetMappedRangeValueClamped(FVector2D(600.0f, 1000.0f), FVector2D(0.0f, 1.0f), FMath::Abs(WheelState.SlipMagnitude));
				const float SkidValue = FMath::GetMappedRangeValueClamped(FVector2D(1000.0f, 1500.0f), FVector2D(0.0f, 1.0f), FMath::Abs(WheelState.SkidMagnitude));
				const float CombinedValue = FMath::Clamp(SlipValue + SkidValue, 0.0f, 1.0f);
				SkidParticleComponents[i]->SetNiagaraVariableFloat("SkidMarkOpacity", CombinedValue);
			}
		}
		else {
			// If the particle exists, but we're not skidding or we are in the air, then destroy the particle
			if (IsValid(SkidParticleComponents[i]))
			{
				SkidParticleComponents[i]->SetNiagaraVariableFloat("SmokeSpawnRate", 0.0f);
				SkidParticleComponents[i]->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, true));

				// Remove the particle from the array
				SkidParticleComponents[i] = nullptr;
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE