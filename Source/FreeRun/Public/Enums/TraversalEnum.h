#pragma once

	UENUM(BlueprintType)
	enum class ETraversalState : uint8
	{
		FreeRoam				UMETA(DisplayName = "FreeRoam"),
		ReadyToClimb			UMETA(DisplayName = "ReadyToClimb"),
		Climb					UMETA(DisplayName = "Climb"),
		Mantle					UMETA(DisplayName = "Mantle"),
		Vault					UMETA(DisplayName = "Vault"),
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
	enum class EClimbDirection : uint8
	{
		NoDirection				UMETA(DisplayName = "NoDirection"),
		Left					UMETA(DisplayName = "Left"),
		Right					UMETA(DisplayName = "Right"),
		Forward					UMETA(DisplayName = "Forward"),
		Backward				UMETA(DisplayName = "Backward"),
		ForwardLeft				UMETA(DisplayName = "ForwardLeft"),
		ForwardRight			UMETA(DisplayName = "ForwardRight"),
		BackwardLeft			UMETA(DisplayName = "BackwardLeft"),
		BackwardRight			UMETA(DisplayName = "BackwardRight"),
	};

	UENUM(BlueprintType)
	enum class EClimbStyle : uint8
	{
		BracedClimb				UMETA(DisplayName = "BracedClimb"),
		FreeHang				UMETA(DisplayName = "FreeHang"),
	};
