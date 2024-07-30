// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TraversalComponent.h"
#include "Animation/AnimInstance.h"
#include "TraversalAnimInstance.generated.h"

using namespace TraversalTypes;

UCLASS()
class FREERUN_API UTraversalAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:

	UTraversalAnimInstance() :
	NewTraversalState(ETraversalState::StateFreeRoam),
	NewTraversalAction(ETraversalAction::NoAction),
	NewClimbStyle(EClimbStyle::ClimbStyleBracedClimb),
	NewClimbDirection(EClimbDirection::NoDirection)
	{}
	
	FORCEINLINE void SetTraversalState(ETraversalState NewState) { NewTraversalState = NewState; }
	FORCEINLINE void SetTraversalAction(ETraversalAction NewAction) { NewTraversalAction = NewAction; }
	FORCEINLINE void SetClimbStyle(EClimbStyle NewStyle) { NewClimbStyle = NewStyle; }
	FORCEINLINE void SetClimbDirection(EClimbDirection NewDirection) { NewClimbDirection = NewDirection; }
private:
	ETraversalState NewTraversalState;
	ETraversalAction NewTraversalAction;
	EClimbStyle NewClimbStyle;
	EClimbDirection NewClimbDirection;
};
