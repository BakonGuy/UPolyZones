// Copyright Seven47 Software All Rights Reserved.


#include "PolyZones_Math.h"

int32 UPolyZones_Math::PZ_FMod(float Dividend, float Divisor)
{
	int32 Result = 0;
	if (Divisor != 0.f)
	{
		const float Quotient = Dividend / Divisor;
		Result = (Quotient < 0.f ? -1 : 1) * FMath::FloorToInt(FMath::Abs(Quotient));
	}

	return Result;
}
