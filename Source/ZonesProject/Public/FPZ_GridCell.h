#pragma once

#include "FPZ_GridCell.generated.h"

USTRUCT(BlueprintType)
struct FPZ_GridCellCoord
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	int32 X;

	UPROPERTY()
	int32 Y;

	FPZ_GridCellCoord(int32 aX, int32 aY)
	{
		X = aX;
		Y = aY;
	}
	FPZ_GridCellCoord()
	{
		X = 0; Y = 0;
	}

	FORCEINLINE bool operator==(const FPZ_GridCellCoord& Src) const
	{
		return (X == Src.X) && (Y == Src.Y);
	}

	FORCEINLINE bool operator!=(const FPZ_GridCellCoord& Src) const
	{
		return X != Src.X || Y != Src.Y;
	}

	FPZ_GridCellCoord operator-(const FPZ_GridCellCoord& Src) const
	{
		return FPZ_GridCellCoord(X - Src.X, Y - Src.Y);
	}

	FString ToString()
	{
		return "[" + FString::FromInt(X) + ":" + FString::FromInt(Y) + "]";
	}

	friend FORCEINLINE uint32 GetTypeHash(const FPZ_GridCellCoord& point)
	{
		return FCrc::MemCrc32(&point, sizeof(FPZ_GridCellCoord));
	}

};

UENUM(BlueprintType)
enum class PZ_LOOKUP_CELL_FLAGS : uint8
{
	Within, Outside, OnEdge
};