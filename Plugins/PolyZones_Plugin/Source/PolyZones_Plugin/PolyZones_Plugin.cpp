// Copyright 2022 Seven47 Software. All Rights Reserved.

#include "PolyZones_Plugin.h"

#define LOCTEXT_NAMESPACE "FPolyZones_PluginModule"

void FPolyZones_PluginModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FPolyZones_PluginModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPolyZones_PluginModule, PolyZones_Plugin)
