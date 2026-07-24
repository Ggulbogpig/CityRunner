// Copyright Epic Games, Inc. All Rights Reserved.

#include "BattleGameMode.h"
#include "BattleCharacter.h"
#include "UObject/ConstructorHelpers.h"

ABattleGameMode::ABattleGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter1"));
	///Game/Characters/BP_Character
	///Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter1
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
