// Copyright Seven47 Software All Rights Reserved.

#include "PolyZone.h"

#include "PolyZones_Math.h"
#include "Components/BillboardComponent.h"

// Sets default values
APolyZone::APolyZone()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bRunConstructionScriptOnDrag = false; // Allow spline editing without the lag

	// Defaults
	CellSize = 50.0f;
	CellsX = 0;
	CellsY = 0;

	CornerDirections.Add( FVector2D(-1.0f, -1.0f) ); // BL
	CornerDirections.Add( FVector2D(1.0f, -1.0f) ); // TL
	CornerDirections.Add( FVector2D(1.0f, 1.0f) ); // TR
	CornerDirections.Add( FVector2D(-1.0f, 1.0f) ); // BR

	// Initializations
	PolySpline = CreateDefaultSubobject<USplineComponent>("PolySpline"); // Root
	RootComponent = PolySpline; // Set actor root before creating other components
	
	PolyIcon = CreateDefaultSubobject<UBillboardComponent>("PolyIcon");
	PolyIcon->SetRelativeLocation(FVector(0.0f,0.0f,50.0f));
	PolyIcon->bUseAttachParentBound = true;
	PolyIcon->SetupAttachment(RootComponent);
}

void APolyZone::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	GridData.Empty();
	if(PolySpline->GetNumberOfSplinePoints() > 2) // Polys have 3 or more edges
	{
		Construct_Spline();
		Construct_SetupGrid();
		PolyZoneConstructed(); // For some reason blueprints construction script has a race condition, so we call our own for now
	}
}

// Construction: Make spline flat and ensure all points are linear
void APolyZone::Construct_Spline()
{
	// Set all points Linear and lock Z
	int LastSplineIndex = PolySpline->GetNumberOfSplinePoints() - 1;
	double PolyActorHeight = GetActorLocation().Z;
	for(int i = 0; i <= LastSplineIndex; i++)
	{
		PolySpline->SetSplinePointType(i, ESplinePointType::Linear, false);
		FVector SplinePoint = PolySpline->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World);
		PolySpline->SetLocationAtSplinePoint(i, FVector(SplinePoint.X,SplinePoint.Y,PolyActorHeight), ESplineCoordinateSpace::World, false);
	}
	
	PolySpline->SetClosedLoop(true, false);
	PolySpline->bInputSplinePointsToConstructionScript = true;
}

void APolyZone::Construct_SetupGrid()
{
	FBoxSphereBounds PolyBounds = PolySpline->CalcBounds(PolySpline->GetComponentTransform());
	
	CellsX = UPolyZones_Math::PZ_FMod( (PolyBounds.BoxExtent.X * 2.0f), CellSize ) + 1;
	CellsY = UPolyZones_Math::PZ_FMod( (PolyBounds.BoxExtent.Y * 2.0f), CellSize ) + 1;

	double OriginX = PolyBounds.Origin.X - (CellsX * 0.5f * CellSize);
	double OriginY = PolyBounds.Origin.Y - (CellsY * 0.5f * CellSize);
	GridOrigin_WS = FVector2D(OriginX, OriginY);

	// Populate grid
	for (int x = 0; x < CellsX-1; ++x)
	{
		for (int y = 0; y < CellsY-1; ++y)
		{
			FPolyZone_GridCell NewCoord = FPolyZone_GridCell(x, y);
			POLYZONE_CELL_FLAGS NewCoordFlag = TestCellAgainstPolygon(NewCoord);
			if(NewCoordFlag != POLYZONE_CELL_FLAGS::Outside) // Unsaved cells can be considered outside the polygon
			{
				GridData.Add(NewCoord, NewCoordFlag);
			}
		}
	}
}

// Copyright (c) 1970-2003, Wm. Randolph Franklin
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// 	Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimers.
// 	Redistributions in binary form must reproduce the above copyright notice in the documentation and/or other materials provided with the distribution.
// 	The name of W. Randolph Franklin may not be used to endorse or promote products derived from this Software without specific prior written permission. 
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
// OR OTHER DEALINGS IN THE SOFTWARE.
// Original Code: https://wrfranklin.org/Research/Short_Notes/pnpoly.html
bool APolyZone::IsPointWithinPolygon(FVector2D TestPoint)
{
	int NumPoints = PolySpline->GetNumberOfSplinePoints();
	
	bool InsidePoly = false;
	for ( int i = 0, j = NumPoints - 1 ; i < NumPoints ; j = i++ )
	{
		FVector Point_i = PolySpline->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World);
		FVector Point_j = PolySpline->GetLocationAtSplinePoint(j, ESplineCoordinateSpace::World);
		if ( ( Point_i.Y > TestPoint.Y ) != ( Point_j.Y > TestPoint.Y ) &&
			 TestPoint.X < ( Point_j.X - Point_i.X ) * ( TestPoint.Y - Point_i.Y ) / ( Point_j.Y - Point_i.Y ) + Point_i.X )
		{
			InsidePoly = !InsidePoly;
		}
	}

	return InsidePoly;
}

FVector2D APolyZone::GetGridCellWorld(const FPolyZone_GridCell& Cell)
{
	return GridOrigin_WS + FVector2D(Cell.X * CellSize, Cell.Y * CellSize);
}

FVector2D APolyZone::GetGridCellCenterWorld(const FPolyZone_GridCell& Cell)
{
	return GetGridCellWorld(Cell) + FVector2D(CellSize*0.5f, CellSize*0.5f);
}

FPolyZone_GridCell APolyZone::GetGridCellAtLocation(FVector Location)
{
	FTransform GridOriginTransform = FTransform( FVector(GridOrigin_WS.X, GridOrigin_WS.Y, 0.0f) );
	FVector LocationLocalToGrid = GridOriginTransform.InverseTransformPosition(Location);
	int32 GridX = FMath::RoundToInt32(LocationLocalToGrid.X / CellSize);
	int32 GridY = FMath::RoundToInt32(LocationLocalToGrid.Y / CellSize);
	return FPolyZone_GridCell(GridX, GridY);
}

FPolyZone_GridCell APolyZone::GetGridCellAtLocation2D(FVector2D Location)
{
	return GetGridCellAtLocation(FVector(Location.X, Location.Y, 0.0f));
}

POLYZONE_CELL_FLAGS APolyZone::GetGridCellFlag(const FPolyZone_GridCell& Cell)
{
	return GridData.FindRef(Cell);
}

POLYZONE_CELL_FLAGS APolyZone::GetFlagAtLocation(FVector Location)
{
	return GetGridCellFlag(GetGridCellAtLocation(Location));
}

POLYZONE_CELL_FLAGS APolyZone::GetFlagAtLocation2D(FVector2D Location)
{
	return GetFlagAtLocation(FVector(Location.X, Location.Y, 0.0f));
}

POLYZONE_CELL_FLAGS APolyZone::TestCellAgainstPolygon(FPolyZone_GridCell Cell)
{
	// Current implementation is an estimate, only tests if each corner is within polygon
	// If all corners are not the same (Within/Outside the polygon) then it's considered on the edge
	// TODO: Implement edge intersection testing
	FVector2D CellCenterLocation_WS = GetGridCellCenterWorld(Cell);

	int Corner = -1;
	bool Result = false;
	bool OnPolyEdge = false;
	for( FVector2D CornerDir : CornerDirections)
	{
		FVector2D CellCorner;
		CellCorner.X = CellCenterLocation_WS.X + (CellSize*0.5f) * CornerDir.X;
		CellCorner.Y = CellCenterLocation_WS.Y + (CellSize*0.5f) * CornerDir.Y;

		bool CurResult = IsPointWithinPolygon(CellCorner);

		Corner++;
		if(Corner == 0) // First test
		{
			Result = CurResult;
		}
		else
		{
			if(CurResult != Result)
			{
				OnPolyEdge = true;
				break;
			}
		}
	}

	if(OnPolyEdge)
	{
		return POLYZONE_CELL_FLAGS::OnEdge;
	}
	if(Result)
	{
		return POLYZONE_CELL_FLAGS::Within;
	}

	return POLYZONE_CELL_FLAGS::Outside;
}

TArray<FPolyZone_GridCell> APolyZone::GetAllGridCells()
{
	TArray<FPolyZone_GridCell> Coords;
	GridData.GenerateKeyArray(Coords);
	return Coords;
}

// Called when the game starts or when spawned
void APolyZone::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APolyZone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
