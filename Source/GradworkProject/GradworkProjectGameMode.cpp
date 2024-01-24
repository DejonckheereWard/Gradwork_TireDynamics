// Copyright Epic Games, Inc. All Rights Reserved.

#include "GradworkProjectGameMode.h"
#include "GradworkProjectPlayerController.h"

AGradworkProjectGameMode::AGradworkProjectGameMode()
{
	PlayerControllerClass = AGradworkProjectPlayerController::StaticClass();
}
