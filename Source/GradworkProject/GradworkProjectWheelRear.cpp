// Copyright Epic Games, Inc. All Rights Reserved.

#include "GradworkProjectWheelRear.h"
#include "UObject/ConstructorHelpers.h"

UGradworkProjectWheelRear::UGradworkProjectWheelRear()
{
	AxleType = EAxleType::Rear;
	bAffectedByHandbrake = true;
	bAffectedByEngine = true;
}