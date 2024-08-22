// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FreeRun/Public/Enums/TraversalEnum.h"
#include "Components/ActorComponent.h"
#include "TraversalComponent.generated.h"

struct FGameplayTag;
class UCapsuleComponent;
class UCharacterMovementComponent;
class UMotionWarpingComponent;
class UCameraComponent;
class UTraversalAnimInstance;
class ADirectionActor;
class UTraversalActionData;

UCLASS(BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
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

	UFUNCTION()
	void OnMontageBlendOut(UAnimMontage* Montage, bool bInterrupted);

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void AddMovementInput(float ScaleValue, bool bIsFront);
	void InitializeReferences(ACharacter* Character, UMotionWarpingComponent* MotionWarpingComponent, UCameraComponent* CameraComponent);
	void TraversalStateSettings(ECollisionEnabled::Type IsEnabled, EMovementMode NewMovementMode, bool bStopMovementImmediately);
	void SetTraversalState(ETraversalState NewState);
	void SetTraversalClimbStyle(EClimbStyle NewStyle);
	void SetTraversalClimbDirection(EClimbDirection NewDirection);
	void TriggerTraversalAction(bool bActionTriggered);
	void DecideTraversalType(bool bActionTriggered);
	void SetTraversalAction(ETraversalAction NewAction);
	void ClearTraversalDatas();
	void ClearMovementDatas();
	void PlayTraversalMontage();
	void ClimbMovement();
	void StopClimbMovement();
	void UpdateClimbLocation(FVector Location, FRotator Rotation);
	void DropFromClimb();

	FORCEINLINE void DeactivateDropping() { bIsDropping = false; }
	FORCEINLINE ETraversalState GetTraversalState() { return TraversalState; }
	
	// IK SECTION
	UFUNCTION(BlueprintCallable)
	void CalculateNextHandClimbLocationIK(const bool bLeftHand);
	UFUNCTION(BlueprintCallable)
	void CalculateNextLegClimbLocationIK(const bool bLeftLeg);
	UFUNCTION(BlueprintCallable)
	void ClimbMovementIK();
	void UpdateHandLocationIK(const bool bLeftHand);
	void UpdateLegLocationIK(const bool bLeftLeg);
	void ResetFootIK();
	
	bool ClimbSideCheck(FVector ImpactPoint);
	void DecideClimbStyle(FVector Location, FRotator Rotation);
	FVector FindWarpLocation(FVector Location, FRotator Rotation, float XOffset, float ZOffset) const;
	
	// WALL IMPLEMENTATIONS
	void GridScanner(int Width, int Height, FVector BaseLocation, FRotator CurrentWorldRotation);
	void CalculateWallMeasures();
	void CalculateWallHeight();
	void CalculateWallDepth();
	void CalculateVaultHeight();


	FHitResult DetectWall();
	float GetClimbStyleValues(EClimbStyle ClimbStyle, float Braced, float Hang);

	bool ValidateAirHang();
	void ValidateIsInLand();
	bool ValidateClimbSurface(FVector ImpactLocation);

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Reference, meta=(AllowPrivateAccess="true"))
	ACharacter* CharacterRef;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Reference, meta=(AllowPrivateAccess="true"))
	UCharacterMovementComponent* MovementComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Reference, meta=(AllowPrivateAccess="true"))
	USkeletalMeshComponent* CharacterMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Reference, meta=(AllowPrivateAccess="true"))
	UCapsuleComponent* CapsuleComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Reference, meta=(AllowPrivateAccess="true"))
	UTraversalAnimInstance* TraversalAnimInstance;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Reference, meta=(AllowPrivateAccess="true"))
	UMotionWarpingComponent* MotionWarping;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Reference, meta=(AllowPrivateAccess="true"))
	UCameraComponent* Camera;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Reference, meta=(AllowPrivateAccess="true"))
	ADirectionActor* DirectionActor;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Reference, meta=(AllowPrivateAccess="true"))
	UTraversalActionData* BracedJumpToClimbData;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Reference, meta=(AllowPrivateAccess="true"))
	UTraversalActionData* FreeHangJumpToClimbData;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Reference, meta=(AllowPrivateAccess="true"))
	UTraversalActionData* BracedFallingClimbData;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Reference, meta=(AllowPrivateAccess="true"))
	UTraversalActionData* FreeHangFallingClimbData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Reference, meta=(AllowPrivateAccess="true"))
	UTraversalActionData* CurrentActionDataRef;
	
	UPROPERTY(VisibleAnywhere)
	ETraversalState TraversalState;
	UPROPERTY(VisibleAnywhere)
	EClimbStyle TraversalClimbStyle;
	UPROPERTY(VisibleAnywhere)
	EClimbDirection TraversalClimbDirection;
	UPROPERTY(VisibleAnywhere)
	ETraversalAction TraversalAction;

	TArray<FHitResult> WallHitsContainer;
	TArray<FHitResult> LineHitsContainer;

	FHitResult WallHitResult;
	FHitResult WallTopResult;
	FHitResult LastWallTopResult;
	FHitResult WallDepthResult;
	FHitResult WallVaultResult;
	FHitResult NextClimbHitResult;
	
	FRotator WallRotation;

	FTimerHandle DroppingTimerHandle;
	
	float WallHeight;
	float WallDepth;
	float VaultHeight;
	float ForwardMovementValue;
	float RightMovementValue;
	float ClimbMoveCheckDistance;
	float ClimbHandSpace;
	
	bool bIsInLand;
	bool bIsDropping;
	
	UPROPERTY(EditDefaultsOnly, meta=(AllowPrivateAccess="true"))
	TSubclassOf<ADirectionActor> DirectionActorClass;
};
