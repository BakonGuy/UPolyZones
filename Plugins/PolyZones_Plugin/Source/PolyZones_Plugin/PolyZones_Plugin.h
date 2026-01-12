// Copyright 2022-2026 Overtorque Creations LLC. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FPolyZones_PluginModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
