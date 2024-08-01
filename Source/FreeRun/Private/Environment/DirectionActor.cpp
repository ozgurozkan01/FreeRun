// Fill out your copyright notice in the Description page of Project Settings.


#include "FreeRun/Public/Environment/DirectionActor.h"

#include "Components/ArrowComponent.h"

// Sets default values
ADirectionActor::ADirectionActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComponent"));
	ArrowComponent->SetHiddenInGame(false);
}

// Called when the game starts or when spawned
void ADirectionActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADirectionActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

