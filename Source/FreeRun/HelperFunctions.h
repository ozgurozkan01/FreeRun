#include "Kismet/KismetMathLibrary.h"

FVector MoveUp(FVector Source, float UpValue)
{
	return Source + FVector(0,0, UpValue);
}

FVector MoveDown(FVector Source, float DownValue)
{
	return Source - FVector(0,0, DownValue);
}

FVector MoveRight(FVector Source, float RightValue, FRotator Rotator)
{
	FVector RightVector = Rotator.Vector().RightVector;
	return Source + (RightVector * RightValue);
}

FVector MoveLeft(FVector Source, float LeftValue, FRotator Rotator)
{
	FVector LeftVector = -Rotator.Vector().RightVector;
	return Source + (LeftVector * LeftValue);
}


FVector MoveForward(FVector Source, float ForwardValue, FRotator Rotator)
{
	FVector ForwardVector = UKismetMathLibrary::GetForwardVector(Rotator);
	return Source + (ForwardVector * ForwardValue);
}

FVector MoveBackward(FVector Source, float BackwardValue, FRotator Rotator)
{
	FVector BackwardVector = -UKismetMathLibrary::GetForwardVector(Rotator);
	return Source + (BackwardVector * BackwardValue);
}

FRotator ReverseNormal(FVector Normal)
{
	FRotator RotFromX = UKismetMathLibrary::MakeRotFromX(Normal);
	FRotator DeltaRotator = RotFromX - FRotator(0, 180.f, 0);
	return FRotator(0, DeltaRotator.Yaw, 0);
}

FRotator AddRotator(FRotator InitialRotator, FRotator AdderRotator)
{
	return InitialRotator + AdderRotator;
}