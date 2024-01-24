// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GradworkProjectPawn.h"
#include "GradworkProjectSportsCar.generated.h"

/**
 *  Sports car wheeled vehicle implementation
 */
UCLASS(abstract)
class GRADWORKPROJECT_API AGradworkProjectSportsCar : public AGradworkProjectPawn
{
	GENERATED_BODY()
	
public:

	AGradworkProjectSportsCar();
};
