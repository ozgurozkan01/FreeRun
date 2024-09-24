// Fill out your copyright notice in the Description page of Project Settings.


#include "FreeRun/Public/Components/TraversalComponent.h"

#include "MotionWarpingComponent.h"
#include "Components/ArrowComponent.h"
#include "FreeRun/Public/Environment/DirectionActor.h"
#include "FreeRun/Public/AnimInstance/TraversalAnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "DataAsset/TraversalActionData.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "FreeRun/Public/Helper/HelperFunctions.h"
#include "Kismet/KismetSystemLibrary.h"

UTraversalComponent::UTraversalComponent() :
	TraversalState(ETraversalState::FreeRoam),
	TraversalClimbStyle(EClimbStyle::BracedClimb),
	TraversalClimbDirection(EClimbDirection::NoDirection),
	TraversalAction(ETraversalAction::NoAction),
	WallRotation(FRotator::ZeroRotator),
	WallHeight(0),
	WallDepth(0),
	VaultHeight(0),
	ForwardMovementValue(0),
	RightMovementValue(0),
	ClimbMoveCheckDistance(10),
	ClimbHandSpace(20),
	HorizontalHopDistance(0),
	VerticalHopDistance(0),
	bIsInLand(true),
	bIsDropping(false)
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UTraversalComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UTraversalComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	/*
	// Get the current location and rotation of DirectionActor
	FVector StartLocation = DirectionActor->ArrowComponent->GetComponentLocation();
	FRotator CurrentRotation = DirectionActor->ArrowComponent->GetComponentRotation();

	// Calculate the end location using the current rotation
	
	FVector EndLocation = StartLocation + UKismetMathLibrary::GetRightVector(CurrentRotation) * 10;

	// Draw the debug line with the updated direction
	DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Orange, false, -1, 0, 1.0f);
	*/

	/*if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(654, -1, FColor::Emerald, FString::Printf(TEXT("%f, %f, %f"), DirectionActor->ArrowComponent->GetComponentRotation().Pitch,
			DirectionActor->ArrowComponent->GetComponentRotation().Yaw,
			DirectionActor->ArrowComponent->GetComponentRotation().Roll));
	}*/
	
	ValidateIsInLand();

	if (bIsInLand && TraversalAction == ETraversalAction::NoAction)
	{
		ClearTraversalDatas();
	}

	else if (!bIsInLand && TraversalState == ETraversalState::FreeRoam && !bIsDropping)
	{
		TriggerTraversalAction(false);
	}

	ClimbMovementIK();

	GEngine->AddOnScreenDebugMessage(654, -1, FColor::Emerald, FString::Printf(TEXT("%s"), *GetControllerDirectionAsString()));
}

void UTraversalComponent::OnMontageBlendOut(UAnimMontage* Montage, bool bInterrupted)
{
	SetTraversalState(CurrentActionDataRef->OutState);
	SetTraversalAction(ETraversalAction::NoAction);
}

void UTraversalComponent::AddMovementInput(float ScaleValue, bool bIsFront)
{
	bIsFront ? ForwardMovementValue = ScaleValue : RightMovementValue = ScaleValue;

	//GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Cyan, FString::Printf(TEXT("RightMovementValue: %f"), RightMovementValue));
	
	switch (TraversalState)
	{
	case ETraversalState::FreeRoam:
		if (bIsFront)
		{
			FVector ForwardVector = UKismetMathLibrary::GetForwardVector(FRotator(0, CharacterRef->GetControlRotation().Yaw, 0));
			CharacterRef->AddMovementInput(ForwardVector, ScaleValue);
		}
		else
		{
			FVector RightVector = UKismetMathLibrary::GetRightVector(FRotator(0, CharacterRef->GetControlRotation().Yaw, CharacterRef->GetControlRotation().Roll));
			CharacterRef->AddMovementInput(RightVector, ScaleValue);
		}
		break;
	case ETraversalState::Climb:
		if (TraversalAnimInstance && TraversalAnimInstance->IsAnyMontagePlaying())
		{
			// GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Cyan, FString::Printf(TEXT("Not Climb Movement")));
			StopClimbMovement();
		}
		else
		{
			// GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Cyan, FString::Printf(TEXT("Climb Movement")));
			ClimbMovement();
		}
		break;
	}
}

void UTraversalComponent::InitializeReferences(ACharacter* Character, UMotionWarpingComponent* MotionWarpingComponent, UCameraComponent* CameraComponent)
{
	if (Character == nullptr || MotionWarpingComponent == nullptr || CameraComponent == nullptr) { return; }

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

	if (TraversalAnimInstance)
	{
		TraversalAnimInstance->OnMontageBlendingOut.AddDynamic(this, &UTraversalComponent::OnMontageBlendOut);
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
		case ETraversalState::FreeRoam:
			TraversalStateSettings(ECollisionEnabled::QueryAndPhysics, MOVE_Walking, false);
			break;
		case ETraversalState::Climb:
			TraversalStateSettings(ECollisionEnabled::NoCollision, MOVE_Flying, true);
			break;
		case ETraversalState::ReadyToClimb:
		case ETraversalState::Mantle:
		case ETraversalState::Vault:
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

float UTraversalComponent::GetClimbStyleValue(EClimbStyle ClimbStyle, float Braced, float Hang)
{
	switch (ClimbStyle)
	{
	case EClimbStyle::BracedClimb:	return Braced;
	case EClimbStyle::FreeHang:		return Hang;
	default: return 0;
	}
}

float UTraversalComponent::GetDirectionValue(EClimbDirection ClimbDirection, FClimbDirectionsValueInfo DirectionInfo)
{
	switch (ClimbDirection) {
	case EClimbDirection::NoDirection:	 return 0;
	case EClimbDirection::Left:			 return DirectionInfo.Left;
	case EClimbDirection::Right:		 return DirectionInfo.Right;
	case EClimbDirection::Forward:		 return DirectionInfo.Forward;
	case EClimbDirection::Backward:		 return DirectionInfo.Backward;
	case EClimbDirection::ForwardLeft:	 return DirectionInfo.ForwardLeft;
	case EClimbDirection::ForwardRight:  return DirectionInfo.ForwardRight;
	case EClimbDirection::BackwardLeft:  return DirectionInfo.BackwardLeft;
	case EClimbDirection::BackwardRight: return DirectionInfo.BackwardRight;
	default: return 0;
	}
}

float UTraversalComponent::GetTraversalStateValues(ETraversalState CurrentState, float Climb, float FreeRoam, float ReadyToClimb)
{
	switch (CurrentState) {
	case ETraversalState::FreeRoam: return FreeRoam;
	case ETraversalState::ReadyToClimb: return ReadyToClimb;
	case ETraversalState::Climb: return Climb;
	default: return 0;
	}
}

float UTraversalComponent::GetCharacterHandHeight()
{
	if (CharacterMesh)
	{
		FVector RightHandLocation = CharacterMesh->GetSocketLocation("hand_r");
		FVector LeftHandLocation = CharacterMesh->GetSocketLocation("hand_l");

		return UKismetMathLibrary::SelectFloat(LeftHandLocation.Z, RightHandLocation.Z, RightHandLocation.Z < LeftHandLocation.Z);
	}

	return 0;
}

bool UTraversalComponent::ValidateAirHang()
{
	if (!NextClimbHitResult.bBlockingHit || CharacterMesh == nullptr) { return false; }

	float ClimbLocationZ = NextClimbHitResult.ImpactPoint.Z;
	float HeadLocationZ = CharacterMesh->GetSocketLocation(FName("head")).Z;

	float Height = HeadLocationZ - ClimbLocationZ;

	if (Height <= 30) { return false; }
	
	return true;
}

void UTraversalComponent::ValidateIsInLand()
{
	if (CharacterMesh)
	{ 
		if (TraversalState != ETraversalState::Climb)
		{
			FVector Start = CharacterMesh->GetSocketLocation("root");
			FVector End = CharacterMesh->GetSocketLocation("root");
			FVector HalfSize{10, 10, 4};
			FRotator Orientation = FRotator::ZeroRotator;
			const TArray<AActor*> ActorsToIgnore;
			FHitResult BoxHitResult;
		
			bIsInLand = UKismetSystemLibrary::BoxTraceSingle(
				this,
				Start,
				End, 
				HalfSize,
				Orientation,
				TraceTypeQuery1,
				false,
				ActorsToIgnore,
				EDrawDebugTrace::ForOneFrame,
				BoxHitResult,
				true);
		
			return;
		}

		bIsInLand = false;
	}
}

bool UTraversalComponent::ValidateClimbSurface(FVector ImpactLocation)
{
	// DrawDebugSphere(GetWorld(), ImpactLocation, 12, 12, FColor::Cyan, false, 5);
	
	FRotator ArrowComRotation = DirectionActor->ArrowComponent->GetComponentRotation();
	//FVector MovedRight = HelperFunc::MoveRight(ImpactLocation, RightMovementValue * 13, ArrowComRotation);
	FVector MovedDown = HelperFunc::MoveDown(ImpactLocation, 90);

	FVector TraceStart = HelperFunc::MoveBackward(MovedDown, 55, ArrowComRotation);
	FVector TraceEnd = HelperFunc::MoveBackward(MovedDown, 55, ArrowComRotation);

	TArray<AActor*> ActorsToIgnore;
	FHitResult SurfaceHit;
	
	bool bCanClimbMode = UKismetSystemLibrary::CapsuleTraceSingle(
		this,
		TraceStart,
		TraceEnd,
		5,
		82,
		TraceTypeQuery1,
		false,
		ActorsToIgnore,
		EDrawDebugTrace::ForOneFrame,
		SurfaceHit,
		true,
		FLinearColor::White,
		FLinearColor::Gray);

	return !bCanClimbMode;
}

bool UTraversalComponent::ValidateMantleSurface()
{
	if (CapsuleComponent == nullptr) { return false; }
	
	float UpValue = CapsuleComponent->GetScaledCapsuleHalfHeight() + 10;
	FVector TraceStart = HelperFunc::MoveUp(WallTopResult.ImpactPoint, UpValue);
	FVector TraceEnd = TraceStart;

	TArray<AActor*> ActorsToIgnore;
	FHitResult MantleHit;

	bool bCanClimb = UKismetSystemLibrary::CapsuleTraceSingle(this,
		TraceStart,
		TraceEnd,
		25,
		CapsuleComponent->GetScaledCapsuleHalfHeight(),
		TraceTypeQuery1,
		false,
		ActorsToIgnore,
		EDrawDebugTrace::ForDuration,
		MantleHit,
		true,
		FLinearColor::White,
FLinearColor::Gray,
	2);
	// If capsule hit something, character cannot climb the wall

	return !bCanClimb;
}

void UTraversalComponent::ClearTraversalDatas()
{
	WallHitResult.Reset();
	WallTopResult.Reset();
	WallDepthResult.Reset();
	WallVaultResult.Reset();
	
	WallHeight = 0;
	WallDepth = 0;
	VaultHeight = 0;
}

void UTraversalComponent::ClearMovementDatas()
{
	RightMovementValue = 0;
	ForwardMovementValue = 0;
	SetTraversalClimbDirection(EClimbDirection::NoDirection);
}

void UTraversalComponent::PlayTraversalMontage()
{
	if (TraversalAnimInstance && CurrentActionDataRef && MotionWarping)
	{
		SetTraversalState(CurrentActionDataRef->InState);
		
		FVector TopLocation = FindWarpLocation(WallTopResult.ImpactPoint, WallRotation, CurrentActionDataRef->Warp1XOffset, CurrentActionDataRef->Warp1ZOffset);
		MotionWarping->AddOrUpdateWarpTargetFromLocationAndRotation(FName("TopResultWarp"), TopLocation, WallRotation);
		
		FVector BalanceLocation = FindWarpLocation(WallTopResult.ImpactPoint, WallRotation, CurrentActionDataRef->Warp2XOffset, CurrentActionDataRef->Warp2ZOffset);
		MotionWarping->AddOrUpdateWarpTargetFromLocationAndRotation(FName("BalanceWarp"), BalanceLocation, WallRotation);

		TraversalAnimInstance->Montage_Play(CurrentActionDataRef->ActionMontage);
		TraversalAnimInstance->Montage_GetBlendingOutDelegate(CurrentActionDataRef->ActionMontage);
	}
}

void UTraversalComponent::ClimbMovement()
{
	FHitResult ClimbWallHitResult;
	FHitResult ClimbTopHitResult;
	
	if (TraversalAction != ETraversalAction::CornerMove)
	{
		if (!(UKismetMathLibrary::Abs(RightMovementValue) > 0.7))
		{
			StopClimbMovement();
		}
		else
		{
			RightMovementValue > 0 ? SetTraversalClimbDirection(EClimbDirection::Right) : SetTraversalClimbDirection(EClimbDirection::Left);

			if (DirectionActor)
			{
				FVector DirectionLocation = DirectionActor->ArrowComponent->GetComponentLocation();

				//FVector DirectionForward = DirectionLocation + DirectionActor->ArrowComponent->GetForwardVector() * 200;
				//DrawDebugLine(GetWorld(), DirectionLocation, DirectionForward, FColor::Black, false, 5);
					
				FRotator DirectionRotation = DirectionActor->ArrowComponent->GetComponentRotation();
				
				for (int i = 0; i < 3; i++)
				{
					int Index = i;
					
					FVector MoveRight = HelperFunc::MoveRight(DirectionLocation, ClimbMoveCheckDistance * RightMovementValue, DirectionRotation);
					FVector TraceStart = HelperFunc::MoveDown(MoveRight, i * 10);
					FVector TraceEnd = HelperFunc::MoveForward(TraceStart, 60, DirectionRotation);
					TArray<AActor*> ActorsToIgnore;

					UKismetSystemLibrary::SphereTraceSingle(this, TraceStart, TraceEnd, 5, TraceTypeQuery1, false, ActorsToIgnore, EDrawDebugTrace::ForOneFrame, ClimbWallHitResult, true, FLinearColor::Black, FLinearColor::Blue, 20);

					if (!ClimbWallHitResult.bStartPenetrating)
					{

						if (ClimbWallHitResult.bBlockingHit)
						{
							// DrawDebugSphere(GetWorld(), ClimbWallHitResult.ImpactPoint, 3, 12, FColor::Purple, false, 2);
							WallRotation = HelperFunc::ReverseNormal(ClimbWallHitResult.ImpactNormal);

							FVector MovedForward = HelperFunc::MoveForward(ClimbWallHitResult.ImpactPoint, 2, WallRotation);
							FVector MovedUp = HelperFunc::MoveUp(MovedForward, 5);
							
							for (int j = 0; j < 7; j++)
							{
								FVector Start = HelperFunc::MoveUp(MovedUp, j * 5);
								FVector End = HelperFunc::MoveDown(Start, 100);

								UKismetSystemLibrary::SphereTraceSingle(this, Start, End, 2.5, TraceTypeQuery1, false, ActorsToIgnore, EDrawDebugTrace::ForOneFrame, ClimbTopHitResult, true, FLinearColor::Red, FLinearColor::Green, 20);

								if (ClimbTopHitResult.bStartPenetrating)
								{
									if (Index == 2 && j == 6)
									{
										StopClimbMovement();
									}
								}
								else
								{
									if (!ClimbTopHitResult.bBlockingHit)
									{
										StopClimbMovement();
									}
									else
									{
										if (!ClimbSideCheck(ClimbTopHitResult.ImpactPoint))
										{
											// DrawDebugSphere(GetWorld(), ClimbWallHitResult.ImpactPoint, 12, 12, FColor::Black);
											if (!ValidateClimbSurface(ClimbWallHitResult.ImpactPoint))
											{
												StopClimbMovement();
											}
											else
											{
												FVector MovedBackward = HelperFunc::MoveBackward(ClimbWallHitResult.ImpactPoint, GetClimbStyleValue(TraversalClimbStyle, 44, 7), WallRotation);
												FVector InputLocation{MovedBackward.X, MovedBackward.Y, ClimbTopHitResult.ImpactPoint.Z};

												DecideClimbStyle(ClimbTopHitResult.ImpactPoint, WallRotation);
												UpdateClimbLocation(InputLocation, WallRotation);
											}
										}
										return; // Break the outter loop.
									}
									break;
								}
							}
							return;
						}
						
						if (Index != 2)
						{
							StopClimbMovement();
						}
					}
				}	
			}
		}
	}
}

void UTraversalComponent::StopClimbMovement()
{
	if (MovementComponent)
	{
		MovementComponent->StopMovementImmediately();
		SetTraversalClimbDirection(EClimbDirection::NoDirection);
	}
}

void UTraversalComponent::UpdateClimbLocation(FVector Location, FRotator Rotation)
{
	// UKismetSystemLibrary::DrawDebugSphere(this, Location, 2, 12, FColor::Cyan);

	FVector CharacterLocation = CharacterRef->GetActorLocation();
	float DeltaSeconds = GetWorld()->GetDeltaSeconds();
	float InterpSpeed = TraversalClimbStyle== EClimbStyle::BracedClimb ? 2.7 : 1.8f;
	
	float X = FMath::FInterpTo(CharacterLocation.X, Location.X, DeltaSeconds, InterpSpeed); // X
	float Y = FMath::FInterpTo(CharacterLocation.Y, Location.Y , DeltaSeconds, InterpSpeed); // Y
	float Z = FMath::FInterpTo(CharacterLocation.Z, Location.Z - GetClimbStyleValue(TraversalClimbStyle, 107.f, 115.f), DeltaSeconds, GetClimbStyleValue(TraversalClimbStyle, 20, 10)); // Z

	FVector NewLocation(X,Y,Z);
	
	CharacterRef->SetActorLocationAndRotation(NewLocation, Rotation);
}

void UTraversalComponent::DropFromClimb()
{
	if (!bIsInLand && TraversalState == ETraversalState::Climb)
	{
		SetTraversalState(ETraversalState::FreeRoam);
		bIsDropping = true;
		GetWorld()->GetTimerManager().SetTimer(DroppingTimerHandle, this, &UTraversalComponent::DeactivateDropping, 0.5f, false);

	}
}

ETraversalAction UTraversalComponent::SelectHopAction()
{
	switch (GetControllerDirection())
	{
	case EClimbDirection::NoDirection:	 return ETraversalAction::NoAction;
	case EClimbDirection::Left:			 return TraversalClimbStyle == EClimbStyle::BracedClimb ? ETraversalAction::BracedClimbHopLeft : ETraversalAction::FreeHangHopLeft;
	case EClimbDirection::Right:		 return TraversalClimbStyle == EClimbStyle::BracedClimb ? ETraversalAction::BracedClimbHopRight : ETraversalAction::FreeHangHopRight;
	case EClimbDirection::Forward:		 return ETraversalAction::BracedClimbHopUp;
	case EClimbDirection::Backward:		 return TraversalClimbStyle == EClimbStyle::BracedClimb ? ETraversalAction::BracedClimbHopDown : ETraversalAction::FreeHangHopDown;
	case EClimbDirection::ForwardLeft:   return TraversalClimbStyle == EClimbStyle::BracedClimb ? ETraversalAction::BracedClimbHopLeftUp : ETraversalAction::FreeHangHopLeft;
	case EClimbDirection::ForwardRight:  return TraversalClimbStyle == EClimbStyle::BracedClimb ? ETraversalAction::BracedClimbHopRightUp : ETraversalAction::FreeHangHopRight;
	case EClimbDirection::BackwardLeft:  return TraversalClimbStyle == EClimbStyle::BracedClimb ? ETraversalAction::BracedClimbHopLeft : ETraversalAction::FreeHangHopLeft;
	case EClimbDirection::BackwardRight: return TraversalClimbStyle == EClimbStyle::BracedClimb ? ETraversalAction::BracedClimbHopRight : ETraversalAction::FreeHangHopRight;
	default: return ETraversalAction::NoAction;
	}
}

void UTraversalComponent::ClimbMovementIK()
{
	if (TraversalState == ETraversalState::Climb)
	{
		UpdateHandLocationIK(true);
		UpdateHandLocationIK(false);
		
		UpdateLegLocationIK(true);
		UpdateLegLocationIK(false);
	}
}

bool UTraversalComponent::ClimbSideCheck(FVector ImpactPoint)
{
	FVector MovedUp = HelperFunc::MoveUp(ImpactPoint, 2);
	
	for (int i = 0; i < 6; i++)
	{
		FVector TraceStart = HelperFunc::MoveUp(MovedUp, i * 5);
		FVector TraceEnd = HelperFunc::MoveRight(TraceStart, RightMovementValue * 15, WallRotation);
		FHitResult ClimbRightResult;
		TArray<AActor*> ActorsToIgnore;
		
		bool bRightHit = UKismetSystemLibrary::LineTraceSingle(this, TraceStart, TraceEnd, TraceTypeQuery1, false, ActorsToIgnore, EDrawDebugTrace::ForOneFrame, ClimbRightResult, true, FLinearColor::Black, FLinearColor::Blue);

		if (bRightHit && i == 5)
		{
			StopClimbMovement();
			return true;
		}

		if (!bRightHit)
		{
			return false;
		}
	}

	return false;
}

void UTraversalComponent::DecideClimbStyle(FVector Location, FRotator Rotation)
{
	FVector MoveDown = HelperFunc::MoveDown(Location, 125.f);
	
	FVector Start = HelperFunc::MoveBackward(MoveDown, 10, Rotation);
	FVector End = HelperFunc::MoveForward(MoveDown, 25, Rotation);
	TArray<AActor*> ActorsToIgnore;
	FHitResult HitResult;
	
	bool bIsTraced = UKismetSystemLibrary::SphereTraceSingle(this, Start, End, 10, TraceTypeQuery1, false, ActorsToIgnore, EDrawDebugTrace::ForOneFrame, HitResult, true, FLinearColor::Red, FLinearColor::Green, 20);

	if (bIsTraced)
	{
		SetTraversalClimbStyle(EClimbStyle::BracedClimb);
	}
	else
	{
		SetTraversalClimbStyle(EClimbStyle::FreeHang);
	}
}

void UTraversalComponent::DecideClimbOrHop()
{
	// Move Forward
	if (GetControllerDirection() == EClimbDirection::Forward ||
  	    GetControllerDirection() == EClimbDirection::ForwardLeft ||
	    GetControllerDirection() == EClimbDirection::ForwardRight
	)
	{
		if (ValidateMantleSurface())
		{
			if (TraversalClimbStyle == EClimbStyle::BracedClimb) { SetTraversalAction(ETraversalAction::BracedClimbClimbUp); }
			else { SetTraversalAction(ETraversalAction::FreeHangClimbUp); }
		}
		else
		{
			CurrentClimbHitResult = WallTopResult;
			CalculateHopLocation();
			DecideClimbStyle(WallTopResult.ImpactPoint, WallRotation);
			NextClimbHitResult = WallTopResult;
			SetTraversalAction(SelectHopAction());
		}
	}
	else
	{
		CurrentClimbHitResult = WallTopResult;
		CalculateHopLocation();
		DecideClimbStyle(WallTopResult.ImpactPoint, WallRotation);
		NextClimbHitResult = WallTopResult;
		SetTraversalAction(SelectHopAction());
	}
}

FVector UTraversalComponent::FindWarpLocation(FVector Location, FRotator Rotation, float XOffset, float ZOffset) const
{
	FVector Forward = HelperFunc::MoveForward(Location, XOffset, Rotation);
	
	return HelperFunc::MoveUp(Forward, ZOffset);
}

void UTraversalComponent::TriggerTraversalAction(bool bActionTriggered)
{
	if (TraversalAction == ETraversalAction::NoAction)
	{
		FHitResult DetectedHit = DetectWall();

		if (!DetectedHit.bBlockingHit) // No Detection
		{
			// GEngine->AddOnScreenDebugMessage(136, 2, FColor::Orange, FString::Printf(TEXT("Wall NOT Detected")));
			if (bActionTriggered)
			{
				CharacterRef->Jump();
			}
		}

		else
		{
			// GEngine->AddOnScreenDebugMessage(136, 2, FColor::Orange, FString::Printf(TEXT("Wall Detected")));
			FRotator CurrentRotation = HelperFunc::ReverseNormal(DetectedHit.ImpactNormal);
			
			float Width = GetTraversalStateValues(TraversalState, 2, 4, 0);
			float Height = GetTraversalStateValues(TraversalState, 7, 30, 0);

			float BaseLocationZ = GetTraversalStateValues(TraversalState, GetCharacterHandHeight(), DetectedHit.ImpactPoint.Z, 0);
			FVector BaseLocation(DetectedHit.ImpactPoint.X, DetectedHit.ImpactPoint.Y, BaseLocationZ);
			
			GridScanner(Width, Height, BaseLocation, CurrentRotation);
			CalculateWallMeasures();
			DecideTraversalType(bActionTriggered);	
		}
	}
}

void UTraversalComponent::DecideTraversalType(bool bActionTriggered)
{
	if (MovementComponent == nullptr) { return; }
	
	if (!WallTopResult.bBlockingHit)
	{
		SetTraversalAction(ETraversalAction::NoAction);
		if (bActionTriggered)
		{
			CharacterRef->Jump();
		}
		return;
	}

	// GEngine->AddOnScreenDebugMessage(136, 2, FColor::Orange, FString::Printf(TEXT("Wall Detected")));
	
	switch (TraversalState)
	{
	case ETraversalState::Climb:
		DecideClimbOrHop();
		break;
	case ETraversalState::FreeRoam:
	{
		if (bIsInLand)
		{
			if (UKismetMathLibrary::InRange_FloatFloat(WallHeight, 90, 160))
			{
				if (UKismetMathLibrary::InRange_FloatFloat(WallDepth, 0, 120))
				{
					if (UKismetMathLibrary::InRange_FloatFloat(VaultHeight, 60, 120))
					{
						if (MovementComponent->Velocity.Length() > 20.f)
						{
							/* VAULT */
						}
						else
						{
							if (ValidateMantleSurface()) { SetTraversalAction(ETraversalAction::Mantle); }
							else						 { SetTraversalAction(ETraversalAction::NoAction); }
						}
					}
					else
					{
						if (ValidateMantleSurface()) { SetTraversalAction(ETraversalAction::Mantle); }
						else						 { SetTraversalAction(ETraversalAction::NoAction); }
					}
				}
				else
				{
					if (ValidateMantleSurface()) { SetTraversalAction(ETraversalAction::Mantle); }
					else						 { SetTraversalAction(ETraversalAction::NoAction); }
				}
				break;
			}

			if (WallHeight < 250.f)
			{
				DecideClimbStyle(WallTopResult.ImpactPoint, WallRotation);

				NextClimbHitResult = WallTopResult;

				if (TraversalClimbStyle == EClimbStyle::BracedClimb)
				{
					SetTraversalAction(ETraversalAction::BracedClimb);
				}
				else
				{
					SetTraversalAction(ETraversalAction::FreeHang);
				}	
			}
			else
			{
				SetTraversalAction(ETraversalAction::NoAction);
			}
		}
		else
		{
			DecideClimbStyle(WallTopResult.ImpactPoint, WallRotation);

			NextClimbHitResult = WallTopResult;

			if (ValidateAirHang())
			{
				if (TraversalClimbStyle == EClimbStyle::BracedClimb)
				{
					SetTraversalAction(ETraversalAction::BracedClimbFallingClimb);
				}
				else
				{
					SetTraversalAction(ETraversalAction::FreeHangFallingClimb);
				}	
			}	
		}
		break;
	}
	}
}

void UTraversalComponent::SetTraversalAction(ETraversalAction NewAction)
{
	if (TraversalAction != NewAction)
	{
		TraversalAction = NewAction;

		if (TraversalAnimInstance)
		{
			TraversalAnimInstance->SetTraversalAction(NewAction);
		}	

		switch (TraversalAction)
		{
		case ETraversalAction::NoAction:
			ClearTraversalDatas(); 
			break;
		case ETraversalAction::BracedClimb: 
			if (BracedJumpToClimbData)
			{
				CurrentActionDataRef = BracedJumpToClimbData;
				PlayTraversalMontage();
			}
			break;
		case ETraversalAction::FreeHang:
			if (FreeHangJumpToClimbData)
			{
				CurrentActionDataRef = FreeHangJumpToClimbData;
				PlayTraversalMontage();
			}
			break;
		case ETraversalAction::FreeHangFallingClimb:
			if (FreeHangFallingClimbData)
			{
				CurrentActionDataRef = FreeHangFallingClimbData;
				PlayTraversalMontage();
			}
			break;
		case ETraversalAction::BracedClimbFallingClimb:
			if (BracedFallingClimbData)
			{
				CurrentActionDataRef = BracedFallingClimbData;
				PlayTraversalMontage();
			}
			break;
		case ETraversalAction::BracedClimbClimbUp:
			if (BracedClimbUpData)
			{
				CurrentActionDataRef = BracedClimbUpData;
				PlayTraversalMontage();
			}
			break;
		case ETraversalAction::FreeHangClimbUp:
			if (FreeHangClimbUpData)
			{
				CurrentActionDataRef = FreeHangClimbUpData;
				PlayTraversalMontage();
			}
			break;
		case ETraversalAction::BracedClimbHopDown:
			if (BracedHopDownData)
			{
				CurrentActionDataRef = BracedHopDownData;
				PlayTraversalMontage();
			}
			break;
		case ETraversalAction::BracedClimbHopUp:
			if (BracedHopUpData)
			{
				CurrentActionDataRef = BracedHopUpData;
				PlayTraversalMontage();
			}
			break;
		case ETraversalAction::BracedClimbHopLeft:
			if (BracedHopLeftData)
			{
				CurrentActionDataRef = BracedHopLeftData;
				PlayTraversalMontage();
			}
			break;
		case ETraversalAction::BracedClimbHopRight:
			if (BracedHopRightData)
			{
				CurrentActionDataRef = BracedHopRightData;
				PlayTraversalMontage();
			}
			break;
		case ETraversalAction::BracedClimbHopRightUp:
			if (BracedHopRightUpData)
			{
				CurrentActionDataRef = BracedHopRightUpData;
				PlayTraversalMontage();
			}
			break;
		case ETraversalAction::BracedClimbHopLeftUp:
			if (BracedHopLeftUpData)
			{
				CurrentActionDataRef = BracedHopLeftUpData;
				PlayTraversalMontage();
			}
			break;
		case ETraversalAction::FreeHangHopDown:
			if (FreeHangHopDownData)
			{
				CurrentActionDataRef = FreeHangHopDownData;
				PlayTraversalMontage();
			}
			break;
		case ETraversalAction::FreeHangHopLeft:
			if (FreeHangHopLeftData)
			{
				CurrentActionDataRef = FreeHangHopLeftData;
				PlayTraversalMontage();
			}
			break;
		case ETraversalAction::FreeHangHopRight:
			if (FreeHangHopRightData)
			{
				CurrentActionDataRef = FreeHangHopRightData;
				PlayTraversalMontage();
			}
			break;
		case ETraversalAction::Mantle:
			if (MantleData)
			{
				CurrentActionDataRef = MantleData;
				PlayTraversalMontage();
			}
			break;
		}
	}
}

void UTraversalComponent::GridScanner(int Width, int Height, FVector BaseLocation, FRotator CurrentWorldRotation)
{
	if (CharacterRef == nullptr) { return; }
	
	WallHitsContainer.Empty();
	
	for (int W = 0; W <= Width; W++)
	{
		LineHitsContainer.Empty();

		float RightValue = (W * 20) - (Width * 10);
		FVector MovedRight = HelperFunc::MoveRight(BaseLocation, RightValue, CurrentWorldRotation);

		for (int H = 0; H < Height; H++)
		{
			int UpValue = H * 8;
			FVector MovedUp = HelperFunc::MoveUp(MovedRight, UpValue);

			FVector StartLocation = HelperFunc::MoveBackward(MovedUp, 60, CurrentWorldRotation);
			FVector EndLocation = HelperFunc::MoveForward(MovedUp, 60, CurrentWorldRotation);

			FHitResult CurrentLineHit;
			TArray<AActor*> ActorsToIgnore;
			
			// UKismetSystemLibrary::SphereTraceSingle(this, StartLocation, EndLocation, 10, TraceTypeQuery1, false, ActorsToIgnore, EDrawDebugTrace::None, CurrentLineHit, true);
			GetWorld()->LineTraceSingleByChannel(CurrentLineHit, StartLocation, EndLocation, ECC_Visibility);

			// UKismetSystemLibrary::DrawDebugLine(this, StartLocation, EndLocation, FColor::Green);
			// UKismetSystemLibrary::DrawDebugSphere(this, CurrentLineHit.ImpactPoint, 2, 12, FColor::Cyan);

			LineHitsContainer.Add(CurrentLineHit);
		}

		for (int i = 1; i < LineHitsContainer.Num(); i++)
		{
			float CurrentDistance = UKismetMathLibrary::Vector_Distance(LineHitsContainer[i].TraceStart, LineHitsContainer[i].TraceEnd);
			float CurrentSelected = UKismetMathLibrary::SelectFloat(LineHitsContainer[i].Distance, CurrentDistance, LineHitsContainer[i].bBlockingHit);

			float PreviousDistance = UKismetMathLibrary::Vector_Distance(LineHitsContainer[i-1].TraceStart, LineHitsContainer[i-1].TraceEnd);
			float PreviousSelected = UKismetMathLibrary::SelectFloat(LineHitsContainer[i-1].Distance, PreviousDistance, LineHitsContainer[i-1].bBlockingHit);
			
		
			float Subtract = CurrentSelected - PreviousSelected;

			if (Subtract > 5)
			{
				WallHitsContainer.Add(LineHitsContainer[i-1]);
				break;
			}
		}
	}
	
	if (!WallHitsContainer.IsEmpty())
	{
		for (int i = 0; i < WallHitsContainer.Num(); i++)
		{
			if (i == 0)
			{
				WallHitResult = WallHitsContainer[i];
			}

			else
			{
				float FirstDistance = UKismetMathLibrary::Vector_Distance(CharacterRef->GetActorLocation(), WallHitResult.ImpactPoint);
				float SecondDistance = UKismetMathLibrary::Vector_Distance(WallHitsContainer[i].ImpactPoint, CharacterRef->GetActorLocation());

				if (SecondDistance <= FirstDistance)
				{
					WallHitResult = WallHitsContainer[i];
				}
			}
		}

		// DrawDebugSphere(GetWorld(), WallHitResult.ImpactPoint, 2, 15, FColor::Red);

		if (WallHitResult.bBlockingHit && !WallHitResult.bStartPenetrating)
		{
			if (TraversalState != ETraversalState::Climb)
			{
				WallRotation = HelperFunc::ReverseNormal(WallHitResult.ImpactNormal);
			}

			for (int i = 0; i < 8; i++)
			{
				FVector MovedForward = HelperFunc::MoveForward(WallHitResult.ImpactPoint, i * 30, WallRotation);
				FVector StartLocation = HelperFunc::MoveUp(MovedForward, 7);
				FVector EndLocation = HelperFunc::MoveDown(StartLocation, 7);
				TArray<AActor*> ActorsToIgnore;
				
				FHitResult TopHitResult;
				bool TraceReturnValue = UKismetSystemLibrary::SphereTraceSingle(this, StartLocation, EndLocation, 2.5, TraceTypeQuery1, false, ActorsToIgnore, EDrawDebugTrace::ForOneFrame, TopHitResult, true);
				// bool LineTraceReturnValue = GetWorld()->LineTraceSingleByChannel(TopHitResult, StartLocation, EndLocation, ECC_Visibility);
				// UKismetSystemLibrary::DrawDebugLine(this, StartLocation, EndLocation, FColor::Green);

				if (i == 0)
				{
					if (TraceReturnValue)
					{
						WallTopResult = TopHitResult;
						DrawDebugSphere(GetWorld(), WallTopResult.ImpactPoint, 5, 15, FColor::Yellow);
					}
				}
				else
				{
					if (TraceReturnValue)
					{
						LastWallTopResult = TopHitResult;	
						// DrawDebugSphere(GetWorld(), LastWallTopResult.ImpactPoint, 5, 15, FColor::Black);
					}
					else
					{
						/* CALCULATE WALL DEPTH */
						if (TraversalState == ETraversalState::FreeRoam)
						{
							FHitResult DepthHitResult;
							FVector DepthStart = HelperFunc::MoveForward(LastWallTopResult.ImpactPoint, 30, WallRotation);
							FVector DepthEnd = LastWallTopResult.ImpactPoint;


							bool bIsDepthHit = UKismetSystemLibrary::SphereTraceSingle(this, DepthStart, DepthEnd, 10, TraceTypeQuery1, false, ActorsToIgnore, EDrawDebugTrace::ForOneFrame, DepthHitResult, true);
							// GetWorld()->LineTraceSingleByChannel(DepthHitResult, DepthStart, DepthEnd, ECC_Visibility)
							if (bIsDepthHit)
							{
								WallDepthResult = DepthHitResult;								
								DrawDebugSphere(GetWorld(), WallDepthResult.ImpactPoint, 5, 15, FColor::White);

								FHitResult VaultHitResult;

								FVector VaultForward = HelperFunc::MoveForward(WallDepthResult.ImpactPoint, 70, WallRotation);
								FVector VaultEnd = HelperFunc::MoveDown(VaultForward, 200);

								bool bIsVaultHit = UKismetSystemLibrary::SphereTraceSingle(this, VaultForward, VaultEnd, 10, TraceTypeQuery1, false, ActorsToIgnore, EDrawDebugTrace::ForOneFrame, VaultHitResult, true);

								// GetWorld()->LineTraceSingleByChannel(VaultHitResult, VaultForward, VaultEnd, ECC_Visibility)
								if (bIsVaultHit)
								{
									WallVaultResult = VaultHitResult;
								} 

								DrawDebugSphere(GetWorld(), WallVaultResult.ImpactPoint, 5, 15, FColor::Magenta);
							}
						}
						break;
					}
				}
			}
		}
	}
}

void UTraversalComponent::CalculateWallMeasures()
{
	CalculateWallHeight();
	CalculateWallDepth();
	CalculateVaultHeight();

	// GEngine->AddOnScreenDebugMessage(132, 2, FColor::Orange, FString::Printf(TEXT("Wall Height  : %f"), WallHeight));
	// GEngine->AddOnScreenDebugMessage(133, 2, FColor::Orange, FString::Printf(TEXT("Wall Depth   : %f"), WallDepth));
	// GEngine->AddOnScreenDebugMessage(134, 2, FColor::Orange, FString::Printf(TEXT("Vault Height : %f"), VaultHeight));
}

void UTraversalComponent::CalculateWallHeight()
{
	if (!WallHitResult.bBlockingHit || !WallTopResult.bBlockingHit)
	{
		WallHeight = 0;
		WallDepth = 0;
		VaultHeight = 0;
		return;
	}

	if (CharacterMesh)
	{
		WallHeight = WallTopResult.ImpactPoint.Z - CharacterMesh->GetSocketLocation("root").Z;
	}
}

void UTraversalComponent::CalculateWallDepth()
{
	if (!WallTopResult.bBlockingHit || !WallDepthResult.bBlockingHit)
	{
		WallDepth = 0;
		return;
	}

	WallDepth = UKismetMathLibrary::Vector_Distance(WallTopResult.ImpactPoint, WallDepthResult.ImpactPoint);
}

void UTraversalComponent::CalculateVaultHeight()
{
	if (!WallDepthResult.bBlockingHit || !WallVaultResult.bBlockingHit)
	{
		VaultHeight = 0;
		return;
	}

	VaultHeight = WallDepthResult.ImpactPoint.Z - WallVaultResult.ImpactPoint.Z;
}

void UTraversalComponent::Calculate2DHopDistance()
{
	FClimbDirectionsValueInfo VerticalDirectionInfo{1, -7.5, -2.5, -2.5, 0.5, 0.5, -4, -4};
	VerticalHopDistance = GetDirectionValue(GetControllerDirection(), VerticalDirectionInfo) * 25.f;

	FClimbDirectionsValueInfo HorizontalDirectionInfo{0, 0, -1, 1, -0.75, 0.75,-0.75, 0.75};
	HorizontalHopDistance = GetDirectionValue(GetControllerDirection(), HorizontalDirectionInfo) * 140.f;
}

void UTraversalComponent::CalculateHopLocation()
{
	Calculate2DHopDistance();

	FVector BaseLocation = HelperFunc::MoveUp(HelperFunc::MoveRight(WallTopResult.ImpactPoint, HorizontalHopDistance, CharacterRef->GetActorRotation()), VerticalHopDistance);
	GridScanner(5, 20, BaseLocation, WallRotation);
}

EClimbDirection UTraversalComponent::GetControllerDirection()
{
	if (ForwardMovementValue == 0 && RightMovementValue > 0) { return EClimbDirection::Right; }
	if (ForwardMovementValue == 0 && RightMovementValue < 0) { return EClimbDirection::Left; }
	if (ForwardMovementValue > 0 && RightMovementValue == 0) { return EClimbDirection::Forward; }
	if (ForwardMovementValue < 0 && RightMovementValue == 0) { return EClimbDirection::Backward; }

	if (ForwardMovementValue < 0 && RightMovementValue < 0) { return EClimbDirection::BackwardLeft; }
	if (ForwardMovementValue < 0 && RightMovementValue > 0) { return EClimbDirection::BackwardRight; }
	if (ForwardMovementValue > 0 && RightMovementValue < 0) { return EClimbDirection::ForwardLeft; }
	if (ForwardMovementValue > 0 && RightMovementValue > 0) { return EClimbDirection::ForwardRight; }

	return EClimbDirection::NoDirection;
}

FString UTraversalComponent::GetControllerDirectionAsString()
{
	switch (GetControllerDirection())
	{
	case EClimbDirection::Left: return "Left";
	case EClimbDirection::Right: return "Right";
	case EClimbDirection::Forward: return "Forward";
	case EClimbDirection::Backward: return "Backward";
	case EClimbDirection::ForwardLeft: return "ForwardLeft";
	case EClimbDirection::ForwardRight: return "ForwardRight";
	case EClimbDirection::BackwardLeft: return "BackwardLeft";
	case EClimbDirection::BackwardRight: return "BackwardRight";
	default: return "NoDirection";
	}
}

void UTraversalComponent::CalculateNextHandClimbLocationIK(const bool bLeftHand)
{
	if (DirectionActor == nullptr) { return ; }
	
	FHitResult ClimbWallHitResult;
	FHitResult ClimbTopHitResult;
	FRotator ClimbHandRotation;
	FVector ClimbHandLocation;
	
	if (TraversalState == ETraversalState::ReadyToClimb)
	{
		if (NextClimbHitResult.bBlockingHit)
		{
			for (int i = 0; i < 5; i++)
			{
				int DirectionMultiplier = bLeftHand ? -1 : 1;
				int Distance = (8 - (i * 2)) * DirectionMultiplier;

				FVector HandLocation = HelperFunc::MoveRight(NextClimbHitResult.ImpactPoint, Distance, WallRotation);

				FVector TraceStart = HelperFunc::MoveBackward(HandLocation, 20, WallRotation);
				FVector TraceEnd = HelperFunc::MoveForward(HandLocation, 20, WallRotation);

				TArray<AActor*> ActorsToIgnore;
				
				UKismetSystemLibrary::SphereTraceSingle(this, TraceStart, TraceEnd, 5,  TraceTypeQuery1, false, ActorsToIgnore, EDrawDebugTrace::ForOneFrame, ClimbWallHitResult, true);

				if (ClimbWallHitResult.bBlockingHit)
				{
					WallRotation = HelperFunc::ReverseNormal(ClimbWallHitResult.ImpactNormal);
					FRotator AddRotation = bLeftHand ? FRotator(90, 0, 200) : FRotator(270, 90, 270);
					ClimbHandRotation = WallRotation + AddRotation - DirectionActor->ArrowComponent->GetComponentRotation();

					for (int j = 0; j < 6; j++)
					{
						FVector MovedForward = HelperFunc::MoveForward(ClimbWallHitResult.ImpactPoint, 2, WallRotation);
						FVector Start = HelperFunc::MoveUp(MovedForward, j * 5);
						FVector End = HelperFunc::MoveDown(Start, 75);

						TArray<AActor*> ActorsToIgnore2;
						
						UKismetSystemLibrary::SphereTraceSingle(this, Start, End, 5,  TraceTypeQuery1, false, ActorsToIgnore2, EDrawDebugTrace::ForOneFrame, ClimbTopHitResult, true);

						if (ClimbTopHitResult.bBlockingHit && !ClimbTopHitResult.bStartPenetrating)
						{
							FVector MovedDown = HelperFunc::MoveDown(ClimbTopHitResult.ImpactPoint, 9);
							ClimbHandLocation = FVector(ClimbWallHitResult.ImpactPoint.X, ClimbWallHitResult.ImpactPoint.Y, MovedDown.Z);

							if (bLeftHand)
							{
								TraversalAnimInstance->SetLeftHandClimbLocation(ClimbHandLocation);
								TraversalAnimInstance->SetLeftHandClimbRotation(ClimbHandRotation);
							}
							else
							{
								TraversalAnimInstance->SetRightHandClimbLocation(ClimbHandLocation);
								TraversalAnimInstance->SetRightHandClimbRotation(ClimbHandRotation);
							}
							return;							
						}
					}
				}
			}
		}
	}
}

void UTraversalComponent::CalculateNextLegClimbLocationIK(const bool bLeftLeg)
{
	if (TraversalClimbStyle == EClimbStyle::BracedClimb)
	{
		for (int i = 0; i < 3; i++)
		{
			FVector MovedUp = HelperFunc::MoveUp(NextClimbHitResult.ImpactPoint, i * 5);

			FVector LegPosition = bLeftLeg ? HelperFunc::MoveLeft(MovedUp, 7, WallRotation) : HelperFunc::MoveRight(MovedUp, 9, WallRotation);

			float DownValue = bLeftLeg ? 150: 140;
			FVector MovedDown = HelperFunc::MoveDown(LegPosition, DownValue);

			FVector TraceStart = HelperFunc::MoveBackward(MovedDown, 30, WallRotation);
			FVector TraceEnd = HelperFunc::MoveForward(MovedDown, 30, WallRotation);

			TArray<AActor*> ActorsToIgnore;
			FHitResult HitResult;
			
			bool bIsHit = UKismetSystemLibrary::SphereTraceSingle(this, TraceStart, TraceEnd, 6, TraceTypeQuery1, false, ActorsToIgnore, EDrawDebugTrace::ForOneFrame, HitResult, true);

			if (bIsHit && TraversalAnimInstance)
			{
				FVector NewLocation = HelperFunc::MoveBackward(HitResult.ImpactPoint, 17, HelperFunc::ReverseNormal(HitResult.ImpactNormal));
				bLeftLeg ? TraversalAnimInstance->SetLeftFootLocation(NewLocation) : TraversalAnimInstance->SetRightFootLocation(NewLocation);
				break;
			}
		}
	}
}

void UTraversalComponent::UpdateHandLocationIK(const bool bLeftHand)
{
	if (CharacterMesh == nullptr || CharacterRef == nullptr || DirectionActor == nullptr) { return; }

	if (TraversalState == ETraversalState::Climb)
	{
		for (int i = 0; i < 5; i++)
		{
			FName SocketName = bLeftHand ? FName("ik_hand_l") : FName("ik_hand_r");
			FVector HandSocketLocation = CharacterMesh->GetSocketLocation(FName(SocketName));

			FVector TracingRepresenter = bLeftHand ? HelperFunc::MoveRight(HandSocketLocation, i * 2 + ClimbHandSpace, CharacterRef->GetActorRotation()) :
												  HelperFunc::MoveLeft(HandSocketLocation, i * 2 + ClimbHandSpace, CharacterRef->GetActorRotation());

			FVector TraceStart = HelperFunc::MoveBackward(TracingRepresenter, 50, CharacterRef->GetActorRotation());
			FVector TraceEnd = HelperFunc::MoveForward(TracingRepresenter, 70, CharacterRef->GetActorRotation());

			TArray<AActor*> ActorsToIgnore;
			FHitResult ClimbWallHitResult;
			
			UKismetSystemLibrary::SphereTraceSingle(this, TraceStart, TraceEnd, 15, TraceTypeQuery1, false, ActorsToIgnore, EDrawDebugTrace::ForOneFrame, ClimbWallHitResult, true);

			if (ClimbWallHitResult.bBlockingHit && !ClimbWallHitResult.bStartPenetrating)
			{
				WallRotation = HelperFunc::ReverseNormal(ClimbWallHitResult.ImpactNormal);
				FRotator AddRotation = bLeftHand ? FRotator(90, 0, 200) : FRotator(270, 90, 270);
				FRotator ClimbHandRotation = WallRotation + AddRotation - DirectionActor->ArrowComponent->GetComponentRotation();

				for (int j = 0; j < 6; j++)
				{
					FVector MovedForward = HelperFunc::MoveForward(ClimbWallHitResult.ImpactPoint, 2, WallRotation);
					FVector Start = HelperFunc::MoveUp(MovedForward, j * 5);
					FVector End = HelperFunc::MoveDown(Start, 75);

					TArray<AActor*> ActorsToIgnore2;
					FHitResult ClimbTopHitResult;
					
					UKismetSystemLibrary::SphereTraceSingle(this, Start, End, 2.5, TraceTypeQuery1, false, ActorsToIgnore, EDrawDebugTrace::ForOneFrame, ClimbTopHitResult, true);

					if (ClimbTopHitResult.bBlockingHit && !ClimbTopHitResult.bStartPenetrating)
					{
						FVector MovedDown2 = HelperFunc::MoveDown(ClimbTopHitResult.ImpactPoint, 9);
						FVector ClimbHandLocation(ClimbWallHitResult.ImpactPoint.X, ClimbWallHitResult.ImpactPoint.Y, MovedDown2.Z);

						if (bLeftHand)
						{
							TraversalAnimInstance->SetLeftHandClimbLocation(ClimbHandLocation);
							TraversalAnimInstance->SetLeftHandClimbRotation(ClimbHandRotation);
						}
						else
						{
							TraversalAnimInstance->SetRightHandClimbLocation(ClimbHandLocation);
							TraversalAnimInstance->SetRightHandClimbRotation(ClimbHandRotation);
						}
						break;
					}
				}
				
				return;
			}
		}
	}
}

void UTraversalComponent::UpdateLegLocationIK(const bool bLeftLeg)
{
	if (TraversalState == ETraversalState::Climb && TraversalAnimInstance && CharacterRef)
	{
		FName CurveName = bLeftLeg ? FName("LeftFootIK") : FName("RightFootIK");
		float CurveValue = TraversalAnimInstance->GetCurveValue(CurveName);

		if (CurveValue == 1) // Braced Climbing
		{
			FVector FootSocketLocation = bLeftLeg ? CharacterMesh->GetSocketLocation(FName("ik_foot_l")) :
											  CharacterMesh->GetSocketLocation(FName("ik_foot_r")); 

			FVector HandLocation = bLeftLeg ? CharacterMesh->GetSocketLocation(FName("hand_l")) :
											  CharacterMesh->GetSocketLocation(FName("hand_r")); 

			FVector FootLocation(FootSocketLocation.X, FootSocketLocation.Y, HandLocation.Z);
			FVector FootLocationDown = HelperFunc::MoveDown(FootLocation, UKismetMathLibrary::SelectFloat(135, 125, bLeftLeg));

			for (int i = 0; i < 3; i++)
			{
				FVector FootMovedUpLocation = HelperFunc::MoveUp(FootLocationDown, i * 5);				

				FVector FootTraceLocation = bLeftLeg ?	HelperFunc::MoveRight(FootMovedUpLocation, 4, CharacterRef->GetActorRotation()) :
														HelperFunc::MoveLeft(FootMovedUpLocation, 4, CharacterRef->GetActorRotation());

				FVector TraceStart = HelperFunc::MoveBackward(FootTraceLocation, 30, CharacterRef->GetActorRotation());
				FVector TraceEnd = HelperFunc::MoveForward(FootTraceLocation, 70, CharacterRef->GetActorRotation());

				TArray<AActor*> ActorsToIgnore;
				FHitResult HitResult;

				UKismetSystemLibrary::SphereTraceSingle(this, TraceStart, TraceEnd, 6, TraceTypeQuery1, false, ActorsToIgnore, EDrawDebugTrace::ForOneFrame, HitResult, true);

				if (HitResult.bBlockingHit && !HitResult.bStartPenetrating)
				{
					bLeftLeg ? TraversalAnimInstance->SetLeftFootLocation(HelperFunc::MoveBackward(HitResult.ImpactPoint, 17, HelperFunc::ReverseNormal(HitResult.ImpactNormal))) :
							   TraversalAnimInstance->SetRightFootLocation(HelperFunc::MoveBackward(HitResult.ImpactPoint, 17, HelperFunc::ReverseNormal(HitResult.ImpactNormal)));
					break;
				}
			}
		}
		else
		{
			ResetFootIK();
		}
	}
}

void UTraversalComponent::ResetFootIK()
{
	if (TraversalAnimInstance && CharacterMesh)
	{
		TraversalAnimInstance->SetRightFootLocation(CharacterMesh->GetSocketLocation(FName("ik_foot_r")));
		TraversalAnimInstance->SetLeftFootLocation(CharacterMesh->GetSocketLocation(FName("ik_foot_l")));
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
			FVector MovedDown = HelperFunc::MoveDown(CharacterLocation, 60);
			FVector MovedUp = HelperFunc::MoveUp(MovedDown,i * 16);

			FVector TraceStart = HelperFunc::MoveBackward(MovedUp, 20, CharacterRotation);
			FVector TraceEnd =	 HelperFunc::MoveForward(MovedUp, 140, CharacterRotation);

			TArray<AActor*> ActorsToIgnore;
			
			UKismetSystemLibrary::SphereTraceSingle(this, TraceStart, TraceEnd, 10, TraceTypeQuery1, false, ActorsToIgnore, EDrawDebugTrace::ForOneFrame, HitResult, true);
			// GetWorld()->SweepSingleByChannel(HitResult, TraceStart, TraceEnd, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(10));
			// UKismetSystemLibrary::DrawDebugLine(this, TraceStart, TraceEnd, FColor::Blue);

			// Wall is identified
			if (HitResult.bBlockingHit && !HitResult.bStartPenetrating)
			{
				// UKismetSystemLibrary::DrawDebugSphere(this, HitResult.ImpactPoint, 5, 12, FColor::Cyan);
				break;
			}
		}
	}

	return HitResult;
}
