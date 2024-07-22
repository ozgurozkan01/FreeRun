// Fill out your copyright notice in the Description page of Project Settings.


#include "TraversalComponent.h"

#include "DirectionActor.h"
#include "TraversalAnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HelperFunctions.h"
#include "Kismet/KismetSystemLibrary.h"

UTraversalComponent::UTraversalComponent() :
	TraversalState(ETraversalState::StateFreeRoam),
	TraversalClimbStyle(EClimbStyle::ClimbStyleBracedClimb),
	TraversalClimbDirection(EClimbDirection::NoDirection),
	TraversalAction(ETraversalAction::NoAction)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UTraversalComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UTraversalComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UTraversalComponent::InitializeReferences(ACharacter* Character, UMotionWarpingComponent* MotionWarpingComponent, UCameraComponent* CameraComponent)
{
	if (Character == nullptr || MotionWarpingComponent == nullptr || CameraComponent == nullptr) { return; }

	UE_LOG(LogTemp, Warning, TEXT("Initialized"));
	
	CharacterRef = Character;
	MovementComponent = CharacterRef->GetCharacterMovement();
	CharacterMesh = CharacterRef->GetMesh();
	CapsuleComponent = CharacterRef->GetCapsuleComponent();
	TraversalAnimInstance = Cast<UTraversalAnimInstance>(CharacterRef->GetMesh()->GetAnimInstance());
	MotionWarping = MotionWarpingComponent;
	Camera = CameraComponent;

	if (DirectionActorClass)
	{
		DirectionActor = GetWorld()->SpawnActor<ADirectionActor>(DirectionActorClass, CharacterRef->GetActorTransform());
		DirectionActor->AttachToComponent(CharacterMesh, FAttachmentTransformRules::SnapToTargetIncludingScale);
		DirectionActor->SetActorRelativeLocation(FVector(0, 0, 195.f));
		DirectionActor->SetActorRelativeRotation(FRotator(0, 90, 0));
	}
}

void UTraversalComponent::TraversalStateSettings(ECollisionEnabled::Type IsEnabled, EMovementMode NewMovementMode, bool bStopMovementImmediately)
{
	if (CapsuleComponent)
	{
		CapsuleComponent->SetCollisionEnabled(IsEnabled);
	}

	if (MovementComponent)
	{
		MovementComponent->SetMovementMode(NewMovementMode);

		if (bStopMovementImmediately)
		{
			MovementComponent->StopMovementImmediately();
		}
	}
}

void UTraversalComponent::SetTraversalState(ETraversalState NewState)
{
	if (TraversalState != NewState)
	{
		TraversalState = NewState;
		TraversalAnimInstance->SetTraversalState(NewState);

		switch (TraversalState)
		{
		case ETraversalState::StateFreeRoam:
			TraversalStateSettings(ECollisionEnabled::QueryAndPhysics, MOVE_Walking, false);
			break;
		case ETraversalState::StateClimb:
			TraversalStateSettings(ECollisionEnabled::NoCollision, MOVE_Flying, true);
			break;
		case ETraversalState::StateReadyToClimb:
		case ETraversalState::StateMantle:
		case ETraversalState::StateVault:
			TraversalStateSettings(ECollisionEnabled::NoCollision, MOVE_Flying, false);
			break;
		}
	}
}

void UTraversalComponent::SetTraversalClimbStyle(EClimbStyle NewStyle)
{
	if (TraversalClimbStyle != NewStyle)
	{
		TraversalClimbStyle = NewStyle;
		TraversalAnimInstance->SetClimbStyle(NewStyle);
	}
}

void UTraversalComponent::SetTraversalClimbDirection(EClimbDirection NewDirection)
{
	if (TraversalClimbDirection != NewDirection)
	{
		TraversalClimbDirection = NewDirection;
		TraversalAnimInstance->SetClimbDirection(NewDirection);
	}
}

float UTraversalComponent::GetClimbStyleValues(EClimbStyle ClimbStyle, float Braced, float Hang)
{
	switch (ClimbStyle)
	{
	case EClimbStyle::ClimbStyleBracedClimb:	return Braced;
	case EClimbStyle::ClimbStyleFreeHang:		return Hang;
	default: return 0;
	}
}

void UTraversalComponent::TriggerTraversalAction(bool bActionTriggered)
{
	if (TraversalAction == ETraversalAction::NoAction)
	{
		DetectWall();
	}
}

FHitResult UTraversalComponent::DetectWall()
{
	FHitResult HitResult;

	if (MovementComponent)
	{
		int32 LastIndex = UKismetMathLibrary::SelectInt(8, 15, MovementComponent->IsFalling());
		for (int i = 0; i < LastIndex; i++)
		{
			FRotator CharacterRotation = CharacterRef->GetActorRotation();
			FVector CharacterLocation = CharacterRef->GetActorLocation();
			FVector MovedDown = MoveDown(CharacterLocation, 60);
			FVector MovedUp = MoveUp(MovedDown,i * 16);

			FVector TraceStart = MoveBackward(MovedUp, 20, CharacterRotation);
			FVector TraceEnd =	 MoveForward(MovedUp, 140, CharacterRotation);

			GetWorld()->SweepSingleByChannel(HitResult, TraceStart, TraceEnd, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(10));
			UKismetSystemLibrary::DrawDebugLine(this, TraceStart, TraceEnd, FColor::Blue);
			
			if (HitResult.bBlockingHit && !HitResult.bStartPenetrating)
			{
				UKismetSystemLibrary::DrawDebugSphere(this, HitResult.ImpactPoint, 10, 12, FColor::Cyan);
				break;
			}
		}
	}

	return HitResult;
}
