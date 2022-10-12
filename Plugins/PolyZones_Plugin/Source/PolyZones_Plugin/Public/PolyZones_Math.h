// Copyright Seven47 Software All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PolyZones_Math.generated.h"

UCLASS()
class UPolyZones_Math : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static int32 PZ_FMod(float Dividend, float Divisor);
	
};
