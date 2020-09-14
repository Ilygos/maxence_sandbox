// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "Gamemodes/Maxence_SandboxGameMode.h"
#include "Characters/Maxence_SandboxCharacter.h"
#include "UObject/ConstructorHelpers.h"

AMaxence_SandboxGameMode::AMaxence_SandboxGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/_Sandbox/Blueprints/BP_SandboxCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
