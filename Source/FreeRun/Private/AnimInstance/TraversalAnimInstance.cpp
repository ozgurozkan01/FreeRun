// Fill out your copyright notice in the Description page of Project Settings.


#include "FreeRun/Public/AnimInstance/TraversalAnimInstance.h"

void UTraversalAnimInstance::SetLeftHandClimbLocation(const FVector NewLocation)
{
	float InterpSpeed = TraversalState == ETraversalState::Climb ? 15.f : 500.f;
	LeftHandClimbLocation = FMath::VInterpTo(LeftHandClimbLocation, NewLocation, GetWorld()->GetDeltaSeconds(), InterpSpeed);
}

void UTraversalAnimInstance::SetRightHandClimbLocation(const FVector NewLocation)
{
	float InterpSpeed = TraversalState == ETraversalState::Climb ? 15.f : 500.f;
	RightHandClimbLocation = FMath::VInterpTo(RightHandClimbLocation, NewLocation, GetWorld()->GetDeltaSeconds(), InterpSpeed);
}

void UTraversalAnimInstance::SetLeftHandClimbRotation(const FRotator NewRotation)
{
	float InterpSpeed = TraversalState == ETraversalState::Climb ? 15.f : 500.f;
	LeftHandClimbRotation = FMath::RInterpTo(LeftHandClimbRotation, NewRotation, GetWorld()->GetDeltaSeconds(), InterpSpeed);
}

void UTraversalAnimInstance::SetRightHandClimbRotation(const FRotator NewRotation)
{
	float InterpSpeed = TraversalState == ETraversalState::Climb ? 15.f : 500.f;
	RightHandClimbRotation = FMath::RInterpTo(RightHandClimbRotation, NewRotation, GetWorld()->GetDeltaSeconds(), InterpSpeed);
}

void UTraversalAnimInstance::SetRightFootLocation(const FVector NewLocation)
{
	float InterpSpeed = TraversalState == ETraversalState::Climb ? 15.f : 500.f;
	RightFootClimbLocation = FMath::VInterpTo(RightFootClimbLocation, NewLocation, GetWorld()->GetDeltaSeconds(), InterpSpeed);
}

void UTraversalAnimInstance::SetLeftFootLocation(const FVector NewLocation)
{
	float InterpSpeed = TraversalState == ETraversalState::Climb ? 15.f : 500.f;
	LeftFootClimbLocation = FMath::VInterpTo(LeftFootClimbLocation, NewLocation, GetWorld()->GetDeltaSeconds(), InterpSpeed);
}
