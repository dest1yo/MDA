// Copyright 2023 dest1yo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"

/**
 * MDA AnimGraph Node Commands
 */
class FMDACommands : public TCommands<FMDACommands>
{
public:
	FMDACommands()
		: TCommands<FMDACommands>(TEXT("MDA"),
			NSLOCTEXT("Contexts", "MDACommands", "MDA Commands"),
			NAME_None,
			FAppStyle::GetAppStyleSetName())
	{}

	virtual void RegisterCommands() override;

public:
	// Blend list options
	TSharedPtr< FUICommandInfo > AddBlendListPin;
	TSharedPtr< FUICommandInfo > RemoveBlendListPin;
};