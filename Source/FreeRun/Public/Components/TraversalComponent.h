// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TraversalComponent.generated.h"

struct FGameplayTag;
class UCapsuleComponent;
class UCharacterMovementComponent;
class UMotionWarpingComponent;
class UCameraComponent;
class UTraversalAnimInstance;
class ADirectionActor;

namespace TraversalTypes
{
	UENUM(BlueprintType)
	enum class ETraversalState	: uint8
	{
		StateFreeRoam			UMETA(DisplayName = "StateFreeRoam"),
		StateReadyToClimb		UMETA(DisplayName = "StateReadyToClimb"),
		StateClimb				UMETA(DisplayName = "StateClimb"),
		StateMantle				UMETA(DisplayName = "StateMantle"),
		StateVault				UMETA(DisplayName = "StateVault"),
	};

	UENUM(BlueprintType)
	enum class ETraversalAction : uint8
	{
		NoAction				UMETA(DisplayName = "NoAction"),
		BracedClimb				UMETA(DisplayName = "BracedClimb"),
		BracedClimbFallingClimb UMETA(DisplayName = "BracedClimbFallingClimb"),
		BracedClimbClimbUp		UMETA(DisplayName = "BracedClimbClimbUp"),
		BracedClimbHopUp		UMETA(DisplayName = "BracedClimbHopUp"),
		BracedClimbHopLeft		UMETA(DisplayName = "BracedClimbHopLeft"),
		BracedClimbHopRight		UMETA(DisplayName = "BracedClimbHopRight"),
		BracedClimbHopLeftUp	UMETA(DisplayName = "BracedClimbHopLeftUp"),
		BracedClimbHopRightUp	UMETA(DisplayName = "BracedClimbHopRightUp"),
		BracedClimbHopDown		UMETA(DisplayName = "BracedClimbHopDown"),
		FreeHang				UMETA(DisplayName = "FreeHang"),
		FreeHangFallingClimb	UMETA(DisplayName = "FreeHangFallingClimb"),
		FreeHangClimbUp			UMETA(DisplayName = "FreeHangClimbUp"),
		FreeHangHopLeft			UMETA(DisplayName = "FreeHangHopLeft"),
		FreeHangHopRight		UMETA(DisplayName = "FreeHangHopRight"),
		FreeHangHopDown			UMETA(DisplayName = "FreeHangHopDown"),
		CornerMove				UMETA(DisplayName = "CornerMove"),
		Mantle					UMETA(DisplayName = "Mantle"),
		Vault					UMETA(DisplayName = "Vault"),
	};

	UENUM(BlueprintType)
	enum class EClimbDirection		: uint8
	{
		NoDirection				UMETA(DisplayName = "NoDirection"),
		DirectionLeft			UMETA(DisplayName = "DirectionLeft"),
		DirectionRight			UMETA(DisplayName = "DirectionRight"),
		DirectionForward		UMETA(DisplayName = "DirectionForward"),
		DirectionBackward		UMETA(DisplayName = "DirectionBackward"),
		DirectionForwardLeft	UMETA(DisplayName = "DirectionForwardLeft"),
		DirectionForwardRight	UMETA(DisplayName = "DirectionForwardRight"),
		DirectionBackwardLeft	UMETA(DisplayName = "DirectionBackwardLeft"),
		DirectionBackwardRight	UMETA(DisplayName = "DirectionBackwardRight"),
	};

	UENUM(BlueprintType)
	enum class EClimbStyle	: uint8
	{
		ClimbStyleBracedClimb	UMETA(DisplayName = "ClimbStyleBracedClimb"),
		ClimbStyleFreeHang		UMETA(DisplayName = "ClimbStyleFreeHang"),
	};
}

using namespace TraversalTypes;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FREERUN_API UTraversalComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTraversalComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void InitializeReferences(ACharacter* Character, UMotionWarpingComponent* MotionWarpingComponent, UCameraComponent* CameraComponent);
	void TraversalStateSettings(ECollisionEnabled::Type IsEnabled, EMovementMode NewMovementMode, bool bStopMovementImmediately);
	void SetTraversalState(ETraversalState NewState);
	void SetTraversalClimbStyle(EClimbStyle NewStyle);
	void SetTraversalClimbDirection(EClimbDirection NewDirection);
	void TriggerTraversalAction(bool bActionTriggered);
	void SetTraversalType(bool JumpAction);
	
	// WALL IMPLEMENTATIONS
	void GridScanner(int Width, int Height, FVector BaseLocation, FRotator CurrentWorldRotation);
	void CalculateWallMeasures();
	void CalculateWallHeight();
	void CalculateWallDepth();
	void CalculateVaultHeight();
	
	FHitResult DetectWall();
	float GetClimbStyleValues(EClimbStyle ClimbStyle, float Braced, float Hang);

private:
	UPROPERTY()
	ACharacter* CharacterRef;
	UPROPERTY()
	UCharacterMovementComponent* MovementComponent;
	UPROPERTY()
	USkeletalMeshComponent* CharacterMesh;
	UPROPERTY()
	UCapsuleComponent* CapsuleComponent;
	UPROPERTY()
	UTraversalAnimInstance* TraversalAnimInstance;
	UPROPERTY()
	UMotionWarpingComponent* MotionWarping;
	UPROPERTY()
	UCameraComponent* Camera;
	UPROPERTY()
	ADirectionActor* DirectionActor;

	ETraversalState TraversalState;
	EClimbStyle TraversalClimbStyle;
	EClimbDirection TraversalClimbDirection;
	ETraversalAction TraversalAction;

	TArray<FHitResult> WallHitsContainer;
	TArray<FHitResult> LineHitsContainer;

	FHitResult WallHitResult;
	FHitResult WallTopResult;
	FHitResult LastWallTopResult;
	FHitResult WallDepthResult;
	FHitResult WallVaultResult;
	
	FRotator WallRotation;

	float WallHeight;
	float WallDepth;
	float VaultHeight;

	bool bIsInLand;
	
	UPROPERTY(EditDefaultsOnly, meta=(AllowPrivateAccess="true"))
	TSubclassOf<ADirectionActor> DirectionActorClass;
};
