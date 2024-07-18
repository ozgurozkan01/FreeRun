// Copyright Epic Games, Inc. All Rights Reserved.

#include "FreeRunGameMode.h"
#include "FreeRunCharacter.h"
#include "UObject/ConstructorHelpers.h"

AFreeRunGameMode::AFreeRunGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
