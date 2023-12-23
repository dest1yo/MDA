// Copyright 2023 dest1yo. All Rights Reserved.

#include "MDACommands.h"

#define LOCTEXT_NAMESPACE "FMDAEditorModule"

void FMDACommands::RegisterCommands()
{
	UI_COMMAND( AddBlendListPin, "Add Blend Pin", "Add blend pin to blend list", EUserInterfaceActionType::Button, FInputChord() )
	UI_COMMAND( RemoveBlendListPin, "Remove Blend Pin", "Remove blend pin", EUserInterfaceActionType::Button, FInputChord() )
}

#undef LOCTEXT_NAMESPACE
