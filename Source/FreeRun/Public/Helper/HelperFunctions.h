#pragma once
#include "Kismet/KismetMathLibrary.h"

namespace HelperFunc
{
	inline FVector MoveUp(FVector Source, float UpValue)
	{
		return Source + FVector(0,0, UpValue);
	}

	inline FVector MoveDown(FVector Source, float DownValue)
	{
		return Source - FVector(0,0, DownValue);
	}

	inline FVector MoveRight(FVector Source, float RightValue, FRotator Rotator)
	{
		FVector RightVector = UKismetMathLibrary::GetRightVector(Rotator);
		return Source + (RightVector * RightValue);
	}

	inline FVector MoveLeft(FVector Source, float LeftValue, FRotator Rotator)
	{
		FVector LeftVector = -UKismetMathLibrary::GetRightVector(Rotator);
		return Source + (LeftVector * LeftValue);
	}
	
	inline FVector MoveForward(FVector Source, float ForwardValue, FRotator Rotator)
	{
		FVector ForwardVector = UKismetMathLibrary::GetForwardVector(Rotator);
		return Source + (ForwardVector * ForwardValue);
	}

	inline FVector MoveBackward(FVector Source, float BackwardValue, FRotator Rotator)
	{
		FVector BackwardVector = -UKismetMathLibrary::GetForwardVector(Rotator);
		return Source + (BackwardVector * BackwardValue);
	}

	inline FRotator ReverseNormal(FVector Normal)
	{
		FRotator RotFromX = UKismetMathLibrary::MakeRotFromX(Normal);
		FRotator DeltaRotator = RotFromX - FRotator(0, 180.f, 0);
		return FRotator(0, DeltaRotator.Yaw, 0);
	}

	inline FRotator AddRotator(FRotator InitialRotator, FRotator AdderRotator)
	{
		return InitialRotator + AdderRotator;
	}
}