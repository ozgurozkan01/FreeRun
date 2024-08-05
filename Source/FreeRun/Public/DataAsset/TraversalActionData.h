// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FreeRun/Public/Enums/TraversalEnum.h"
#include "Engine/DataAsset.h"
#include "TraversalActionData.generated.h"

UCLASS()
class FREERUN_API UTraversalActionData : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Data)
	UAnimMontage* ActionMontage;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Data)
	float Warp1XOffset;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Data)
	float Warp1ZOffset;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Data)
	float Warp2XOffset;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Data)
	float Warp2ZOffset;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Data)
	float Warp3XOffset;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Data)
	float Warp3ZOffset;
	UPROPERTY(EditAnywhere, Category=Data)
	ETraversalState InState;  // Current Traversal State
	UPROPERTY(EditAnywhere, Category=Data)
	ETraversalState OutState; // Next Traversal State
};