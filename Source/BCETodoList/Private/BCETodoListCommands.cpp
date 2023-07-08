// Copyright Epic Games, Inc. All Rights Reserved.

#include "BCETodoListCommands.h"

#define LOCTEXT_NAMESPACE "FBCETodoListModule"

void FBCETodoListCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "Todo List", "Bring up the Todo list", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
