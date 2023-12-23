// Copyright 2023 dest1yo. All Rights Reserved.

#include "MDARuntime.h"

#define LOCTEXT_NAMESPACE "FMDARuntimeModule"

void FMDARuntimeModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FMDARuntimeModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FMDARuntimeModule, MDARuntime)