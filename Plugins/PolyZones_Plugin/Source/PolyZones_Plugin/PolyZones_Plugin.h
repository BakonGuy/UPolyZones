// Copyright 2022 Seven47 Software. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FPolyZones_PluginModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
