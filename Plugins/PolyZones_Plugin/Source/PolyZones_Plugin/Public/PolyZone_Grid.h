#pragma once

#include "PolyZone_Grid.generated.h"

USTRUCT(BlueprintType)
struct FPolyZone_GridCell
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 X;

	UPROPERTY(BlueprintReadOnly)
	int32 Y;

	FPolyZone_GridCell(int32 aX, int32 aY)
	{
		X = aX;
		Y = aY;
	}
	FPolyZone_GridCell()
	{
		X = 0; Y = 0;
	}

	FORCEINLINE bool operator==(const FPolyZone_GridCell& Src) const
	{
		return (X == Src.X) && (Y == Src.Y);
	}

	FORCEINLINE bool operator!=(const FPolyZone_GridCell& Src) const
	{
		return X != Src.X || Y != Src.Y;
	}

	FPolyZone_GridCell operator-(const FPolyZone_GridCell& Src) const
	{
		return FPolyZone_GridCell(X - Src.X, Y - Src.Y);
	}

	FString ToString()
	{
		return "[" + FString::FromInt(X) + ":" + FString::FromInt(Y) + "]";
	}

	friend FORCEINLINE uint32 GetTypeHash(const FPolyZone_GridCell& point)
	{
		return FCrc::MemCrc32(&point, sizeof(FPolyZone_GridCell));
	}

};

UENUM(BlueprintType)
enum class POLYZONE_CELL_FLAGS : uint8
{
	Outside, Within, OnEdge // Outside is the default because cells outside the grid return the default
};