// Copyright Seven47 Software All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PerformanceTool.generated.h"

/**
 * 
 */
UCLASS()
class ZONESPROJECT_API UPerformanceTool : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintCallable, Category = "Performance")
	static double GetCurrentTimeSeconds();

	UFUNCTION(BlueprintCallable, Category = "Performance")
	static double MakeElapsedTimeMs(double StartSeconds, double EndSeconds);
	
};
