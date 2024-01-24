// Copyright Epic Games, Inc. All Rights Reserved.

#include "GradworkProjectWheelFront.h"
#include "UObject/ConstructorHelpers.h"

UGradworkProjectWheelFront::UGradworkProjectWheelFront()
{
	AxleType = EAxleType::Front;
	bAffectedBySteering = true;
	MaxSteerAngle = 40.f;
}