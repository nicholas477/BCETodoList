// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "BCETodoListStyle.h"

class FBCETodoListCommands : public TCommands<FBCETodoListCommands>
{
public:

	FBCETodoListCommands()
		: TCommands<FBCETodoListCommands>(TEXT("BCETodoList"), NSLOCTEXT("Contexts", "BCETodoList", "BCETodoList Plugin"), NAME_None, FBCETodoListStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};