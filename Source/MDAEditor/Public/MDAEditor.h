// Copyright 2023 dest1yo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

class FBlueprintEditor;
/**
 * MDA Editor module
 */
class FMDAEditorModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	/** IModuleInterface implementation End*/

	/**
	 * Singleton-like access to this module's interface.  This is just for convenience!
	 * Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
	 *
	 * @return Returns singleton instance, loading the module on demand if needed
	 */
	static FORCEINLINE FMDAEditorModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FMDAEditorModule>(TEXT("MDAEditor"));
	}

	/**
	 * Checks to see if this module is loaded and ready.  It is only valid to call Get() if IsAvailable() returns true.
	 *
	 * @return True if the module is loaded and ready to use
	 */
	static FORCEINLINE bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded(TEXT("MDAEditor"));
	}

	void OnCreateMDACommands();

	FORCEINLINE TSharedPtr<FUICommandList> GetMDACommandList() { return MDACommandList; }

	// Get current focused blueprint editor
	static FBlueprintEditor* GetFocusedBlueprintEditor();
	// Get current focused graph editor
	static TSharedPtr<SGraphEditor> GetFocusedGraphEditor();

	// Pose pin UI handlers
	// Add
	void OnAddPosePin();
	bool CanAddPosePin() const { return true;};
	// Remove
	void OnRemovePosePin();
	bool CanRemovePosePin() const { return true;};
	
	void TestCommand();

private:
	// MDA Command List
	TSharedPtr<FUICommandList> MDACommandList;
};



