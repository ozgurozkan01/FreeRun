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

USTRUCT()
struct FClimbDirectionsValueInfo
{
	GENERATED_BODY()

	float Forward;
	float Backward;
	float Left;
	float Right;
	float ForwardLeft;
	float ForwardRight;
	float BackwardLeft;
	float BackwardRight;
};

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

	void GridScanner(int Width, int Height, FVector BaseLocation, FRotator CurrentWorldRotation);
	void AddMovementInput(float ScaleValue, bool bIsFront);
	void InitializeReferences(ACharacter* Character, UMotionWarpingComponent* MotionWarpingComponent, UCameraComponent* CameraComponent);
	void TraversalStateSettings(ECollisionEnabled::Type IsEnabled, EMovementMode NewMovementMode, bool bStopMovementImmediately);
	void SetTraversalState(ETraversalState NewState);
	void SetTraversalClimbStyle(EClimbStyle NewStyle);
	void SetTraversalClimbDirection(EClimbDirection NewDirection);
	void TriggerTraversalAction(bool bActionTriggered);
	void SetTraversalAction(ETraversalAction NewAction);
	void ClearTraversalDatas();
	void ClearMovementDatas();
	void PlayTraversalMontage();
	void ClimbMovement();
	void StopClimbMovement();
	void DropFromClimb();
	ETraversalAction SelectHopAction();
	
	void DecideTraversalType(bool bActionTriggered);
	void DecideClimbStyle(FVector Location, FRotator Rotation);
	void DecideClimbOrHop();
	void DecideClimbActionBasedOnAirHang();
	
	FORCEINLINE void DeactivateDropping() { bIsDropping = false; }
	FORCEINLINE ETraversalState GetTraversalState() { return TraversalState; }
	
	UFUNCTION(BlueprintCallable)
	void ClimbMovementIK();
	void UpdateHandLocationIK(const bool bLeftHand);
	void UpdateLegLocationIK(const bool bLeftLeg);
	void UpdateClimbLocation(FVector Location, FRotator Rotation);
	void ResetFootIK();
	
	bool ClimbSideCheck(FVector ImpactPoint);
	FVector FindWarpLocation(FVector Location, FRotator Rotation, float XOffset, float ZOffset) const;
	EClimbDirection GetControllerDirection();
	FString GetControllerDirectionAsString();

	UFUNCTION(BlueprintCallable)
	void CalculateNextHandClimbLocationIK(const bool bLeftHand);
	UFUNCTION(BlueprintCallable)
	void CalculateNextLegClimbLocationIK(const bool bLeftLeg);
	void CalculateWallMeasures();
	void CalculateWallHeight();
	void CalculateWallDepth();
	void CalculateVaultHeight();
	void Calculate2DHopDistance();
	void CalculateHopLocation();
	
	FHitResult DetectWall();
	float GetClimbStyleValue(EClimbStyle ClimbStyle, float Braced, float Hang);
	float GetDirectionValue(EClimbDirection ClimbDirection, FClimbDirectionsValueInfo DirectionInfo);
	float GetTraversalStateValues(ETraversalState CurrentState, float Climb, float FreeRoam, float ReadyToClimb);
	float GetCharacterHandHeight();
	
	bool ValidateClimbMovementSurface(FVector ImpactLocation);
	bool ValidateAirHang();
	void ValidateIsInLand();
	bool ValidateMantleSurface();
	bool ValidateVaultSurface();
	bool ValidateClimbSurface(FVector Location, FRotator Rotation);
	
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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Reference, meta=(AllowPrivateAccess="true"))
	UTraversalActionData* BracedClimbUpData;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Reference, meta=(AllowPrivateAccess="true"))
	UTraversalActionData* FreeHangClimbUpData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Reference, meta=(AllowPrivateAccess="true"))
	UTraversalActionData* BracedHopDownData;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Reference, meta=(AllowPrivateAccess="true"))
	UTraversalActionData* BracedHopLeftData;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Reference, meta=(AllowPrivateAccess="true"))
	UTraversalActionData* BracedHopRightData;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Reference, meta=(AllowPrivateAccess="true"))
	UTraversalActionData* BracedHopLeftUpData;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Reference, meta=(AllowPrivateAccess="true"))
	UTraversalActionData* BracedHopRightUpData;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Reference, meta=(AllowPrivateAccess="true"))
	UTraversalActionData* BracedHopUpData;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Reference, meta=(AllowPrivateAccess="true"))
	UTraversalActionData* FreeHangHopDownData;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Reference, meta=(AllowPrivateAccess="true"))
	UTraversalActionData* FreeHangHopLeftData;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Reference, meta=(AllowPrivateAccess="true"))
	UTraversalActionData* FreeHangHopRightData;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Reference, meta=(AllowPrivateAccess="true"))
	UTraversalActionData* MantleData;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Reference, meta=(AllowPrivateAccess="true"))
	UTraversalActionData* VaultData;
	
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
	FHitResult CurrentClimbHitResult;
	
	FRotator WallRotation;

	FTimerHandle DroppingTimerHandle;
	
	float WallHeight;
	float WallDepth;
	float VaultHeight;
	float ForwardMovementValue;
	float RightMovementValue;
	float ClimbMoveCheckDistance;
	float ClimbHandSpace;
	float HorizontalHopDistance;
	float VerticalHopDistance;
	
	bool bIsInLand;
	bool bIsDropping;
	
	UPROPERTY(EditDefaultsOnly, meta=(AllowPrivateAccess="true"))
	TSubclassOf<ADirectionActor> DirectionActorClass;
};
