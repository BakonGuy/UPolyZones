// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PZ_Math.generated.h"

UCLASS()
class ZONESPROJECT_API UPZ_Math : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static int32 PZ_FMod(float Dividend, float Divisor)
	{
		int32 Result = 0;
		if (Divisor != 0.f)
		{
			const float Quotient = Dividend / Divisor;
			Result = (Quotient < 0.f ? -1 : 1) * FMath::FloorToInt(FMath::Abs(Quotient));
		}

		return Result;
	};
	
};
