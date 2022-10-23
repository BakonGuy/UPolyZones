// Copyright Seven47 Software All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PolyZone_Interface.generated.h"

// the UINTERFACE class is not the actual interface. It is an empty class that exists only for visibility to Unreal Engine's reflection system.
// The actual interface that will be inherited by other classes must have the same class name, but with the initial "U" changed to an "I". 
UINTERFACE(MinimalAPI, Blueprintable)
class UPolyZone_Interface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class POLYZONES_PLUGIN_API IPolyZone_Interface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintImplementableEvent, Category = "PolyZone")
	void EnterPolyZone(APolyZone* PolyZone);

	UFUNCTION(BlueprintImplementableEvent, Category = "PolyZone")
	void ExitPolyZone(APolyZone* PolyZone);
};