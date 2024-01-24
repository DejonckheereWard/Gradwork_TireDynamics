// Copyright Epic Games, Inc. All Rights Reserved.


#include "GradworkProjectUI.h"

void UGradworkProjectUI::UpdateSpeed(float NewSpeed)
{
	// format the speed to KPH or MPH
	float FormattedSpeed = FMath::Abs(NewSpeed) * (bIsMPH ? 0.022f : 0.036f);

	// call the Blueprint handler
	OnSpeedUpdate(FormattedSpeed);
}

void UGradworkProjectUI::UpdateGear(int32 NewGear)
{
	// call the Blueprint handler
	OnGearUpdate(NewGear);
}

void UGradworkProjectUI::UpdateRpm(float NewRpm)
{
	OnRpmUpdate(NewRpm);
}

