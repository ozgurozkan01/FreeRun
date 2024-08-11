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
	WallHeight(0),
	WallDepth(0),
	VaultHeight(0),
	ForwardMovementValue(0),
	RightMovementValue(0),
	ClimbMoveCheckDistance(10),
	bIsInLand(true)
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

	ValidateIsInLand();

	if (bIsInLand)
	{
		if (TraversalAction == ETraversalAction::NoAction)
		{
			ClearTraversalDatas();
		}
	}

	else
	{
		/* In Air */
		if (TraversalState == ETraversalState::FreeRoam)
		{
			TriggerTraversalAction(false);
		}
	}
}

void UTraversalComponent::AddMovementInput(float ScaleValue, bool bIsFront)
{
	bIsFront ? ForwardMovementValue = ScaleValue : RightMovementValue = ScaleValue;

	GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Cyan, FString::Printf(TEXT("RightMovementValue: %f"), RightMovementValue));
	
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
			StopClimbMovement();
		}
		else
		{
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

float UTraversalComponent::GetClimbStyleValues(EClimbStyle ClimbStyle, float Braced, float Hang)
{
	switch (ClimbStyle)
	{
	case EClimbStyle::BracedClimb:	return Braced;
	case EClimbStyle::FreeHang:		return Hang;
	default: return 0;
	}
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
	FRotator ArrowComRotation = DirectionActor->ArrowComponent->GetComponentRotation();
	FVector MovedRight = HelperFunc::MoveRight(ImpactLocation, RightMovementValue * 13, ArrowComRotation);
	FVector MovedDown = HelperFunc::MoveDown(MovedRight, 90);

	FVector TraceStart = HelperFunc::MoveBackward(MovedDown, 40, CharacterRef->GetActorRotation());
	FVector TraceEnd = HelperFunc::MoveBackward(MovedDown, 25, CharacterRef->GetActorRotation());

	TArray<AActor*> ActorsToIgnore;
	FHitResult SurfaceHit;
	
	bool bCanClimbMode = UKismetSystemLibrary::CapsuleTraceSingle(this, TraceStart, TraceEnd, 5, 82, TraceTypeQuery1, false, ActorsToIgnore, EDrawDebugTrace::ForOneFrame, SurfaceHit, true);

	return !bCanClimbMode;
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

void UTraversalComponent::PlayTraversalMontage(const UTraversalActionData* CurrentActionData)
{
	if (TraversalAnimInstance && CurrentActionData && MotionWarping)
	{
		SetTraversalState(CurrentActionData->InState);

		FVector TopLocation = FindWwarpLocation(WallTopResult.ImpactPoint, WallRotation, CurrentActionData->Warp1XOffset, CurrentActionData->Warp1ZOffset);
		MotionWarping->AddOrUpdateWarpTargetFromLocationAndRotation(FName("TopResultWarp"), TopLocation, WallRotation);

		FVector BalanceLocation = FindWwarpLocation(WallTopResult.ImpactPoint, WallRotation, CurrentActionData->Warp2XOffset, CurrentActionData->Warp2ZOffset);
		MotionWarping->AddOrUpdateWarpTargetFromLocationAndRotation(FName("BalanceWarp"), BalanceLocation, WallRotation);

		TraversalAnimInstance->Montage_Play(CurrentActionData->ActionMontage);

		SetTraversalState(CurrentActionData->OutState);
		SetTraversalAction(ETraversalAction::NoAction);
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
				for (int i = 0; i < 3; i++)
				{
					int Index = i;

					FVector DirectionLocation = DirectionActor->ArrowComponent->GetComponentLocation();
					FRotator DirectionRotation = DirectionActor->ArrowComponent->GetComponentRotation();
					
					FVector MoveRight = HelperFunc::MoveRight(DirectionLocation, ClimbMoveCheckDistance * RightMovementValue, DirectionRotation);
					FVector TraceStart = HelperFunc::MoveDown(MoveRight, i * 10);
					FVector TraceEnd = HelperFunc::MoveForward(TraceStart, 60, DirectionRotation);

					TArray<AActor*> ActorsToIgnore;
					UKismetSystemLibrary::SphereTraceSingle(this, TraceStart, TraceEnd, 5, TraceTypeQuery1, false, ActorsToIgnore, EDrawDebugTrace::ForOneFrame, ClimbWallHitResult, true, FLinearColor::Black, FLinearColor::Blue, 20);

					if (!ClimbWallHitResult.bStartPenetrating)
					{
						if (ClimbWallHitResult.bBlockingHit)
						{
							WallRotation = HelperFunc::ReverseNormal(ClimbWallHitResult.ImpactNormal);
							
							for (int j = 0; j < 7; j++)
							{
								FVector MovedForward = HelperFunc::MoveForward(ClimbWallHitResult.ImpactPoint, 2, WallRotation);
								FVector MovedUp = HelperFunc::MoveUp(MovedForward, 5);
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
												FVector MovedBackward = HelperFunc::MoveBackward(ClimbWallHitResult.ImpactPoint, GetClimbStyleValues(TraversalClimbStyle, 35, 7), WallRotation);
												FVector InputLocation{MovedBackward.X, MovedBackward.Y, ClimbTopHitResult.ImpactPoint.Z};
												UpdateClimbLocation(InputLocation, WallRotation);
												DecideClimbStyle(ClimbTopHitResult.ImpactPoint, WallRotation);
											}
										}
										return; // Break the outter loop.
									}
									break;
								}
							}
						}
						else
						{
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
	FVector CharacterLocation = CharacterRef->GetActorLocation();
	float DeltaSeconds = GetWorld()->GetDeltaSeconds();
	float InterpSpeed = 2.f;
	
	float X = FMath::FInterpTo(CharacterLocation.X, Location.X, DeltaSeconds, InterpSpeed); // X
	float Y = FMath::FInterpTo(CharacterLocation.Y, Location.Y, DeltaSeconds, InterpSpeed); // Y
	float Z = FMath::FInterpTo(CharacterLocation.Z, Location.Z - GetClimbStyleValues(TraversalClimbStyle, 107.f, 115.f), DeltaSeconds, GetClimbStyleValues(TraversalClimbStyle, 2.7, 1.8)); // Z

	CharacterRef->SetActorLocationAndRotation(FVector(X,Y,Z), Rotation);
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
	
	bool bIsTraced = UKismetSystemLibrary::SphereTraceSingle(this, Start, End, 10, TraceTypeQuery1, false, ActorsToIgnore, EDrawDebugTrace::None, HitResult, true, FLinearColor::Red, FLinearColor::Green, 20);

	if (bIsTraced)
	{
		SetTraversalClimbStyle(EClimbStyle::BracedClimb);
	}
	else
	{
		SetTraversalClimbStyle(EClimbStyle::FreeHang);
	}
}

FVector UTraversalComponent::FindWwarpLocation(FVector Location, FRotator Rotation, float XOffset, float ZOffset) const
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
			GridScanner(4, 30, DetectedHit.ImpactPoint, CurrentRotation);
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

	switch (TraversalState)
	{
	case ETraversalState::Climb:
			
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
							GEngine->AddOnScreenDebugMessage(135, 2, FColor::Orange, FString::Printf(TEXT("VAULT")));
							/* VAULT */
						}
						else
						{
							GEngine->AddOnScreenDebugMessage(135, 2, FColor::Orange, FString::Printf(TEXT("MANTLE 1")));
							/* MANTLE */
						}
					}
					else
					{
						GEngine->AddOnScreenDebugMessage(135, 2, FColor::Orange, FString::Printf(TEXT("MANTLE 2")));
						/* MANTLE */
					}
				}
				else
				{
					GEngine->AddOnScreenDebugMessage(135, 2, FColor::Orange, FString::Printf(TEXT("MANTLE 3")));
					/* MANTLE */
				}
				break;
			}

			if (WallHeight < 250.f)
			{
				GEngine->AddOnScreenDebugMessage(135, 2, FColor::Orange, FString::Printf(TEXT("CLIMB")));
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
				// CurrentActionData = BracedJumpToClimbData;
				PlayTraversalMontage(BracedJumpToClimbData);
			}
			break;
		case ETraversalAction::FreeHang:
			if (FreeHangJumpToClimb)
			{
				// CurrentActionData = FreeHangJumpToClimb;
				PlayTraversalMontage(FreeHangJumpToClimb);
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
			UKismetSystemLibrary::DrawDebugSphere(this, CurrentLineHit.ImpactPoint, 2, 12, FColor::Cyan);

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

		DrawDebugSphere(GetWorld(), WallHitResult.ImpactPoint, 2, 15, FColor::Red);

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
				bool TraceReturnValue = UKismetSystemLibrary::SphereTraceSingle(this, StartLocation, EndLocation, 2.5, TraceTypeQuery1, false, ActorsToIgnore, EDrawDebugTrace::None, TopHitResult, true);
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
						DrawDebugSphere(GetWorld(), LastWallTopResult.ImpactPoint, 5, 15, FColor::Black);
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

	GEngine->AddOnScreenDebugMessage(132, 2, FColor::Orange, FString::Printf(TEXT("Wall Height  : %f"), WallHeight));
	GEngine->AddOnScreenDebugMessage(133, 2, FColor::Orange, FString::Printf(TEXT("Wall Depth   : %f"), WallDepth));
	GEngine->AddOnScreenDebugMessage(134, 2, FColor::Orange, FString::Printf(TEXT("Vault Height : %f"), VaultHeight));
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
			
			UKismetSystemLibrary::SphereTraceSingle(this, TraceStart, TraceEnd, 10, TraceTypeQuery1, false, ActorsToIgnore, EDrawDebugTrace::None, HitResult, true);
			// GetWorld()->SweepSingleByChannel(HitResult, TraceStart, TraceEnd, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(10));
			// UKismetSystemLibrary::DrawDebugLine(this, TraceStart, TraceEnd, FColor::Blue);

			// Wall is identified
			if (HitResult.bBlockingHit && !HitResult.bStartPenetrating)
			{
				UKismetSystemLibrary::DrawDebugSphere(this, HitResult.ImpactPoint, 5, 12, FColor::Cyan);
				break;
			}
		}
	}

	return HitResult;
}
