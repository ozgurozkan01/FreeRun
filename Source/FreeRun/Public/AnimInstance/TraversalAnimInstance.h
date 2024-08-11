// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FreeRun/Public/Components/TraversalComponent.h"
#include "Animation/AnimInstance.h"
#include "TraversalAnimInstance.generated.h"

class AFreeRunCharacter;

UCLASS()
class FREERUN_API UTraversalAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:

	UTraversalAnimInstance()  :
	TraversalState(ETraversalState::FreeRoam),
	TraversalAction(ETraversalAction::NoAction),
	ClimbStyle(EClimbStyle::BracedClimb),
	ClimbDirection(EClimbDirection::NoDirection)
	{
	}

	FORCEINLINE ETraversalState GetTraversalState() const { return TraversalState; }
	FORCEINLINE ETraversalAction GetTraversalAction() const { return TraversalAction; }
	FORCEINLINE EClimbStyle GetClimbStyle() const { return ClimbStyle; }
	FORCEINLINE EClimbDirection GetClimbDirection() const { return ClimbDirection; }
	
	FORCEINLINE void SetTraversalState(ETraversalState NewState) { TraversalState = NewState; }
	FORCEINLINE void SetTraversalAction(ETraversalAction NewAction) { TraversalAction = NewAction; }
	FORCEINLINE void SetClimbStyle(EClimbStyle NewStyle) { ClimbStyle = NewStyle; }
	FORCEINLINE void SetClimbDirection(EClimbDirection NewDirection) { ClimbDirection = NewDirection; }
private:
	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	ETraversalState TraversalState;
	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	ETraversalAction TraversalAction;
	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	EClimbStyle ClimbStyle;
	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	EClimbDirection ClimbDirection;
};
