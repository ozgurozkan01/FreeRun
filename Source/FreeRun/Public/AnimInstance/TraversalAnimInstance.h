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
	ClimbDirection(EClimbDirection::NoDirection),
	LeftHandClimbLocation(0),
	RightHandClimbLocation(0),
	LeftHandClimbRotation(0),
	RightHandClimbRotation(0)
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

	UFUNCTION(BlueprintCallable)
	void SetLeftHandClimbLocation(const FVector NewLocation);
	UFUNCTION(BlueprintCallable)
	void SetRightHandClimbLocation(const FVector NewLocation);
	UFUNCTION(BlueprintCallable)
	void SetLeftHandClimbRotation(const FRotator NewRotation);
	UFUNCTION(BlueprintCallable)
	void SetRightHandClimbRotation(const FRotator NewRotation);
	UFUNCTION(BlueprintCallable)
	void SetRightFootLocation(const FVector NewLocation);
	UFUNCTION(BlueprintCallable)
	void SetLeftFootLocation(const FVector NewLocation);
private:
	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	ETraversalState TraversalState;
	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	ETraversalAction TraversalAction;
	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	EClimbStyle ClimbStyle;
	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	EClimbDirection ClimbDirection;

	/* Be used to change hand location and rotation of hand as dynamically */
	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	FVector LeftHandClimbLocation;
	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	FVector RightHandClimbLocation;
	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	FRotator LeftHandClimbRotation;
	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	FRotator RightHandClimbRotation;
	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	FVector LeftFootClimbLocation;
	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	FVector RightFootClimbLocation;
};
