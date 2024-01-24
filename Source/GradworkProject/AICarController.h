// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AICarController.generated.h"

class UInputMappingContext;
class AGradworkProjectPawn;

/**
 * 
 */
UCLASS()
class GRADWORKPROJECT_API AAICarController : public AAIController
{
	GENERATED_BODY()

protected:
	/** Pointer to the controlled vehicle pawn */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<AGradworkProjectPawn> VehiclePawn;

	/** Time before starting the AI, editable **/
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float StartDelay = 0.0f;
	
public:
	virtual void BeginPlay() override;

	virtual void Tick(float Delta) override;


protected:

	virtual void OnPossess(APawn* InPawn) override;
	
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
    void StartSequence();

};
