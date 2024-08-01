// Fill out your copyright notice in the Description page of Project Settings.


#include "FreeRun/Public/Components/TraversalComponent.h"

#include "FreeRun/Public/Environment/DirectionActor.h"
#include "FreeRun/Public/AnimInstance/TraversalAnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "FreeRun/Public/Helper/HelperFunctions.h"
#include "AI/NavigationSystemBase.h"
#include "Kismet/KismetSystemLibrary.h"

UTraversalComponent::UTraversalComponent() :
	TraversalState(ETraversalState::StateFreeRoam),
	TraversalClimbStyle(EClimbStyle::ClimbStyleBracedClimb),
	TraversalClimbDirection(EClimbDirection::NoDirection),
	TraversalAction(ETraversalAction::NoAction),
	WallHeight(0),
	WallDepth(0),
	VaultHeight(0),
	bIsInLand(true)
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
		CalculateWallMeasures();
		SetTraversalType(bActionTriggered);
	}
}

void UTraversalComponent::SetTraversalType(bool JumpAction)
{
	/* NO ACTION */
	if (!WallTopResult.bBlockingHit)
	{
		if (JumpAction)
		{
			CharacterRef->Jump();
		}
		return;
	}

	switch (TraversalState)
	{
	case ETraversalState::StateClimb:
			
		break;
	case ETraversalState::StateFreeRoam:
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
				/* CLIMB STATE */
			}
		}	
		break;
	}
	}
}

void UTraversalComponent::GridScanner(int Width, int Height, FVector BaseLocation, FRotator CurrentWorldRotation)
{
	UE_LOG(LogTemp, Warning, TEXT("test"));
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
			if (TraversalState != ETraversalState::StateClimb)
			{
				WallRotation = HelperFunc::ReverseNormal(WallHitResult.ImpactNormal);
			}

			for (int i = 0; i < 8; i++)
			{
				FVector MovedForward = HelperFunc::MoveForward(WallHitResult.ImpactPoint, i * 30, WallRotation);
				FVector StartLocation = HelperFunc::MoveUp(MovedForward, 7);
				FVector EndLocation = HelperFunc::MoveDown(StartLocation, 7);

				FHitResult TopHitResult;
				bool LineTraceReturnValue = GetWorld()->LineTraceSingleByChannel(TopHitResult, StartLocation, EndLocation, ECC_Visibility);
				// UKismetSystemLibrary::DrawDebugLine(this, StartLocation, EndLocation, FColor::Green);

				if (i == 0)
				{
					if (LineTraceReturnValue)
					{
						WallTopResult = TopHitResult;
						DrawDebugSphere(GetWorld(), WallTopResult.ImpactPoint, 5, 15, FColor::Yellow);
					}
				}
				else
				{
					if (LineTraceReturnValue)
					{
						LastWallTopResult = TopHitResult;	
						DrawDebugSphere(GetWorld(), LastWallTopResult.ImpactPoint, 5, 15, FColor::Black);
					}
					else
					{
						if (TraversalState == ETraversalState::StateFreeRoam)
						{
							FHitResult DepthHitResult;
							FVector DepthStart = HelperFunc::MoveForward(LastWallTopResult.ImpactPoint, 30, WallRotation);
							FVector DepthEnd = LastWallTopResult.ImpactPoint;
							if (GetWorld()->LineTraceSingleByChannel(DepthHitResult, DepthStart, DepthEnd, ECC_Visibility))
							{
								WallDepthResult = DepthHitResult;								
							}
						}

						DrawDebugSphere(GetWorld(), WallDepthResult.ImpactPoint, 5, 15, FColor::White);

						FVector VaultForward = HelperFunc::MoveForward(WallDepthResult.ImpactPoint, 70, WallRotation);

						FHitResult VaultHitResult;
						FVector VaultEnd = HelperFunc::MoveDown(VaultForward, 200);

						if (GetWorld()->LineTraceSingleByChannel(VaultHitResult, VaultForward, VaultEnd, ECC_Visibility))
						{
							WallVaultResult = VaultHitResult;
						}

						DrawDebugSphere(GetWorld(), WallVaultResult.ImpactPoint, 5, 15, FColor::Magenta);
						
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
	
	WallHeight = WallTopResult.ImpactPoint.Z - CharacterRef->GetMesh()->GetSocketLocation("root").Z;
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

			GetWorld()->SweepSingleByChannel(HitResult, TraceStart, TraceEnd, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(10));
			UKismetSystemLibrary::DrawDebugLine(this, TraceStart, TraceEnd, FColor::Blue);
			
			if (HitResult.bBlockingHit && !HitResult.bStartPenetrating)
			{
				UKismetSystemLibrary::DrawDebugSphere(this, HitResult.ImpactPoint, 10, 12, FColor::Purple);
				break;
			}
		}
	}

	return HitResult;
}
