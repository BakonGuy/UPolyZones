// Fill out your copyright notice in the Description page of Project Settings.


#include "ZonesProject/Public/PolyZone.h"

#include "Components/BillboardComponent.h"
#include "ZonesProject/Public/PZ_Math.h"

// Sets default values
APolyZone::APolyZone()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

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
	GridData.Empty();
	if(PolySpline->GetNumberOfSplinePoints() > 2) // Polys have 3 or more lines
	{
		Construct_Spline();
		Construct_SetupGrid();
	}
	Super::OnConstruction(Transform);
	// Construction always seeming a state behind is probably because the bounds are not computed until after construction
	// TODO: Compute spline bounds manually
}

// Construction: Make spline flat and ensure all points are linear
void APolyZone::Construct_Spline()
{
	// Set all points Linear and lock Z
	int LastSplineIndex = PolySpline->GetNumberOfSplinePoints() - 1;
	double PolyActorHeight = GetActorLocation().Z;
	for(int i = 0; i <= LastSplineIndex; i++)
	{
		PolySpline->SetSplinePointType(i, ESplinePointType::Linear);
		FVector SplinePoint = PolySpline->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World);
		PolySpline->SetLocationAtSplinePoint(i, FVector(SplinePoint.X,SplinePoint.Y,PolyActorHeight), ESplineCoordinateSpace::World);
	}
	
	PolySpline->SetClosedLoop(true);
	PolySpline->bInputSplinePointsToConstructionScript = true;
}

void APolyZone::Construct_SetupGrid()
{
	FBoxSphereBounds PolyBounds =  PolySpline->Bounds;
	
	CellsX = UPZ_Math::PZ_FMod( (PolyBounds.BoxExtent.X * 2.0f), CellSize ) + 1;
	CellsY = UPZ_Math::PZ_FMod( (PolyBounds.BoxExtent.Y * 2.0f), CellSize ) + 1;

	double OriginX = PolyBounds.Origin.X - (CellsX * 0.5f * CellSize);
	double OriginY = PolyBounds.Origin.Y - (CellsY * 0.5f * CellSize);
	GridOrigin_WS = FVector2D(OriginX, OriginY);

	// Populate grid
	for (int x = 0; x < CellsX-1; ++x)
	{
		for (int y = 0; y < CellsY-1; ++y)
		{
			FPZ_GridCellCoord NewCoord = FPZ_GridCellCoord(x, y);
			PZ_LOOKUP_CELL_FLAGS NewCoordFlag = TestCellAgainstPolygon(NewCoord);
			//if(NewCoordFlag != PZ_LOOKUP_CELL_FLAGS::Outside) // Unsaved cells can be considered outside the polygon
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

FVector2D APolyZone::GetGridCellWorld(const FPZ_GridCellCoord& Cell)
{
	return GridOrigin_WS + FVector2D(Cell.X * CellSize, Cell.Y * CellSize);
}

FVector2D APolyZone::GetGridCellCenterWorld(const FPZ_GridCellCoord& Cell)
{
	return GetGridCellWorld(Cell) + FVector2D(CellSize*0.5f, CellSize*0.5f);
}

PZ_LOOKUP_CELL_FLAGS APolyZone::GetGridCellFlag(const FPZ_GridCellCoord& Cell)
{
	return GridData.FindRef(Cell);
}

PZ_LOOKUP_CELL_FLAGS APolyZone::TestCellAgainstPolygon(FPZ_GridCellCoord Cell)
{
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
		return PZ_LOOKUP_CELL_FLAGS::OnEdge;
	}
	if(Result)
	{
		return PZ_LOOKUP_CELL_FLAGS::Within;
	}

	return PZ_LOOKUP_CELL_FLAGS::Outside;
}

TArray<FPZ_GridCellCoord> APolyZone::GetAllGridCells()
{
	TArray<FPZ_GridCellCoord> Coords;
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
