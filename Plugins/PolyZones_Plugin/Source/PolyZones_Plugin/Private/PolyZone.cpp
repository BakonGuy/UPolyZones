// Copyright 2022-2026 Overtorque Creations LLC. All Rights Reserved.

#include "PolyZone.h"

#if WITH_EDITORONLY_DATA
#include "PolyZone_Visualizer.h"
#endif

#include "Runtime/Launch/Resources/Version.h"
#include "PolyZone_Interface.h"
#include "Components/BillboardComponent.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

// Sets default values
APolyZone::APolyZone()
{
	PrimaryActorTick.bCanEverTick = true; // Tick

	OverlapTypes.Add(ECollisionChannel::ECC_Pawn);

	// Grid defaults
	CornerDirections.Add(FVector2D(-1.0f, -1.0f)); // BL
	CornerDirections.Add(FVector2D(1.0f, -1.0f)); // TL
	CornerDirections.Add(FVector2D(1.0f, 1.0f)); // TR
	CornerDirections.Add(FVector2D(-1.0f, 1.0f)); // BR

	// Initializations
	USceneComponent* SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComp"));
	SetRootComponent(SceneComponent); // Set actor root before creating other components
	RootComponent->Mobility = EComponentMobility::Static;

	PolySpline = CreateDefaultSubobject<USplineComponent>("PolySpline");
	PolySpline->SetupAttachment(RootComponent);

	#if WITH_EDITORONLY_DATA // Editor only defaults
	bRunConstructionScriptOnDrag = false; // Allow spline editing without the lag

	PolyIcon = CreateEditorOnlyDefaultSubobject<UBillboardComponent>("PolyIcon");
	if( PolyIcon )
	{
		PolyIcon->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));
		PolyIcon->SetupAttachment(RootComponent);
	}
	#endif
	SetCanBeDamaged(false);
}

void APolyZone::OnConstruction(const FTransform& Transform)
{
	Build_PolyZone();
	PolyZoneConstructed(); // For some reason blueprints construction script has a race condition, so we call our own for now
	Super::OnConstruction(Transform);
}

// Called when the game starts or when spawned
void APolyZone::BeginPlay()
{
	Super::BeginPlay();

	// Reconstruct needed data
	Construct_Bounds();
	Build_PolyZone();
	#if WITH_EDITORONLY_DATA
	Construct_Visualizer();
	#endif

	// Initialize actor tracking
	if( IsValid(BoundsOverlap) )
	{
		// Bind Overlap Events
		BoundsOverlap->OnComponentBeginOverlap.AddDynamic(this, &APolyZone::OnBeginBoundsOverlap);
		BoundsOverlap->OnComponentEndOverlap.AddDynamic(this, &APolyZone::OnEndBoundsOverlap);

		// Track pre-spawned actors
		TArray<AActor*> StartingOverlaps;
		BoundsOverlap->GetOverlappingActors(StartingOverlaps);
		int LastIndex = StartingOverlaps.Num() - 1;
		for( int Index = 0; Index <= LastIndex; ++Index )
		{
			if( StartingOverlaps[Index]->Implements<UPolyZone_Interface>() )
			{
				TrackedActors.Add(StartingOverlaps[Index], false);
			}
		}
	}
}

void APolyZone::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	SetActorTickEnabled(false);

	TArray<TPair<AActor*, bool>> TrackedActorsArray;

	// Save map as array, so changes won't effect the loop
	for( const TPair<AActor*, bool>& MapPair : TrackedActors )
	{
		TrackedActorsArray.Add(MapPair);
	}
	TrackedActors.Empty();

	// Notify tracked actors
	for( const TPair<AActor*, bool>& MapPair : TrackedActorsArray )
	{
		AActor* TrackedActor = MapPair.Key;
		bool IsWithinPoly = MapPair.Value;
		if( IsWithinPoly && IsValid(TrackedActor) )
		{
			PolyZoneOverlapChange(TrackedActor, false);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void APolyZone::K2_DestroyActor()
{
	// Delays destroy by 1 tick, if called via blueprint
	WantsDestroyed = true;
}

// Called every frame
void APolyZone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if( WantsDestroyed )
	{
		Destroy();
		return;
	}

	if( ActorTracking ) DoActorTracking();
	if( bDebugGrid ) DrawDebugGrid();
}

// Rebuilds the PolyZone (can be run during runtime)
void APolyZone::Build_PolyZone()
{
	if( PolySpline->GetNumberOfSplinePoints() < 3 ) // A polygon must have at least 3 points to be valid
	{
		// Create a default polygon
		TArray<FVector> DefaultPoints;
		DefaultPoints.Add(FVector(0, 0, 0));
		DefaultPoints.Add(FVector(100, -50, 0));
		DefaultPoints.Add(FVector(100, 50, 0));
		PolySpline->SetSplinePoints(DefaultPoints, ESplineCoordinateSpace::Local, true);
	}
	if( PolySpline->GetNumberOfSplinePoints() >= 3 ) // We check again because sometimes the default is overridden
	{
		Construct_Polygon();
		Construct_Bounds();
		Construct_SetupGrid();
		Construct_Visualizer();
	}
}

void APolyZone::Construct_Polygon()
{
	Polygon.Empty(); // Can rebuild at runtime
	Polygon2D.Empty();

	// Make spline flat and ensure all points are linear
	int LastSplineIndex = PolySpline->GetNumberOfSplinePoints() - 1;
	double ActorHeight = GetActorLocation().Z;
	for( int i = 0; i <= LastSplineIndex; i++ )
	{
		PolySpline->SetSplinePointType(i, ESplinePointType::Linear, false);
		FVector SplinePoint = PolySpline->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World);
		SplinePoint.Z = ActorHeight;
		PolySpline->SetLocationAtSplinePoint(i, SplinePoint, ESplineCoordinateSpace::World, false);
		Polygon.Add(SplinePoint);
		Polygon2D.Add(FVector2D(SplinePoint.X, SplinePoint.Y));
	}

	PolySpline->SetUnselectedSplineSegmentColor(FLinearColor(0, 1, 0)); // Make spline green
	PolySpline->SetClosedLoop(true, false);
	PolySpline->bInputSplinePointsToConstructionScript = true;
	PolySpline->UpdateSpline(); // Call after making all our edits

	// Save calculated bounds to save cpu cycles in PolyZone test
	Bounds_MinX = Polygon[0].X;
	Bounds_MaxX = Polygon[0].X;
	Bounds_MinY = Polygon[0].Y;
	Bounds_MaxY = Polygon[0].Y;
	for( int i = 1; i < LastSplineIndex + 1; i++ )
	{
		FVector q = Polygon[i];
		Bounds_MinX = FMath::Min(q.X, Bounds_MinX);
		Bounds_MaxX = FMath::Max(q.X, Bounds_MaxX);
		Bounds_MinY = FMath::Min(q.Y, Bounds_MinY);
		Bounds_MaxY = FMath::Max(q.Y, Bounds_MaxY);
	}
}

void APolyZone::Construct_Bounds()
{
	// TODO (Oct22/2022) : Calculate smallest rectangular bounds for overlap
	PolyBounds = PolySpline->CalcBounds(PolySpline->GetComponentTransform());
	UBoxComponent* NewBoundsOverlap = NewObject<UBoxComponent>(this);
	NewBoundsOverlap->CreationMethod = EComponentCreationMethod::UserConstructionScript;
	if( NewBoundsOverlap )
	{
		// Create
		FVector OverlapExtent = FVector(PolyBounds.BoxExtent.X, PolyBounds.BoxExtent.Y, ZoneHeight * 0.5f);
		NewBoundsOverlap->SetAbsolute(true, true, true); // Ignore relative transform
		NewBoundsOverlap->RegisterComponent();
		NewBoundsOverlap->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

		#if WITH_EDITORONLY_DATA
		// Setup Visualization
		NewBoundsOverlap->ShapeColor = FColor(0, 255, 0);
		NewBoundsOverlap->SetVisibility(ShowVisualization);
		NewBoundsOverlap->SetHiddenInGame(!bDebugGrid);
		#endif

		// Setup Shape
		NewBoundsOverlap->SetBoxExtent(OverlapExtent);
		NewBoundsOverlap->SetWorldLocation(PolyBounds.Origin + FVector(0, 0, ZoneHeight * 0.5f));

		// Setup Collision
		NewBoundsOverlap->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		NewBoundsOverlap->SetCollisionObjectType(ZoneObjectType);
		NewBoundsOverlap->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		for( ECollisionChannel ToOverlap : OverlapTypes )
		{
			NewBoundsOverlap->SetCollisionResponseToChannel(ToOverlap, ECollisionResponse::ECR_Overlap);
		}
	}
	BoundsOverlap = NewBoundsOverlap;
}

void APolyZone::Construct_SetupGrid()
{
	GridData.Empty(); // Can rebuild at runtime

	int32 NumPoints = PolySpline->GetNumberOfSplinePoints();
	UsesGrid = (NumPoints >= 6);
	if( UsesGrid )
	{
		// Calculate a performant cell size
		float DesiredCellCount = FMath::Min(40.0f, 2.0f * NumPoints);
		float DistanceToCover = FMath::Max(PolyBounds.BoxExtent.X * 2.0f, PolyBounds.BoxExtent.Y * 2.0f);
		CellSize = DistanceToCover / DesiredCellCount;

		FVector2D BoundsBottomLeft;
		BoundsBottomLeft.X = PolyBounds.Origin.X - PolyBounds.BoxExtent.X;
		BoundsBottomLeft.Y = PolyBounds.Origin.Y - PolyBounds.BoxExtent.Y;
		FVector2D BoundsTopRight;
		BoundsTopRight.X = PolyBounds.Origin.X + PolyBounds.BoxExtent.X;
		BoundsTopRight.Y = PolyBounds.Origin.Y + PolyBounds.BoxExtent.Y;

		// Find the cell (world grid) for the bottom left bounds, and make it our local grid origin
		const double NewOriginX = FMath::FloorToDouble(BoundsBottomLeft.X / CellSize) * CellSize;
		const double NewOriginY = FMath::FloorToDouble(BoundsBottomLeft.Y / CellSize) * CellSize;
		GridOrigin = FVector(NewOriginX, NewOriginY, GetActorLocation().Z);

		float DistanceX = BoundsTopRight.X - GridOrigin.X;
		float DistanceY = BoundsTopRight.Y - GridOrigin.Y;

		// Find how many cells we will need to cover the polygon
		GridCellsX = FMath::CeilToInt(DistanceX / CellSize);
		GridCellsY = FMath::CeilToInt(DistanceY / CellSize);

		const int32 TotalCells = GridCellsX * GridCellsY;
		GridData.Init(POLYZONE_CELL_FLAGS::Outside, TotalCells);

		// Populate grid data
		for( int32 GridX = 0; GridX < GridCellsX; GridX++ )
		{
			for( int32 GridY = 0; GridY < GridCellsY; GridY++ )
			{
				FPolyZone_GridCell NewCell = FPolyZone_GridCell(GridX, GridY);
				POLYZONE_CELL_FLAGS FlagForNewCell = TestCellAgainstPolygon(NewCell);
				const int32 CellIndex = GetGridCellIndex(NewCell);
				if( CellIndex != INDEX_NONE )
				{
					GridData[CellIndex] = FlagForNewCell;
				}
			}
		}
	}
}

void APolyZone::Construct_Visualizer()
{
	#if WITH_EDITORONLY_DATA
	if( ShowVisualization )
	{
		UChildActorComponent* NewVisualizer = NewObject<UChildActorComponent>(this);
		if( NewVisualizer )
		{
			NewVisualizer->CreationMethod = EComponentCreationMethod::UserConstructionScript;
			NewVisualizer->RegisterComponent();
			NewVisualizer->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
			NewVisualizer->SetChildActorClass(APolyZone_Visualizer::StaticClass());
		}
		EditorVisualizer = NewVisualizer;

		if( IsValid(EditorVisualizer) )
		{
			EditorVisualizer->SetWorldTransform(FTransform(FVector(0, 0, GetActorLocation().Z))); // Move XY to world origin
			EditorVisualizer->CreateChildActor();
			AActor* VizActor = EditorVisualizer->GetChildActor();
			APolyZone_Visualizer* Viz = Cast<APolyZone_Visualizer>(VizActor);
			if( IsValid(Viz) )
			{
				Viz->SetActorHiddenInGame(HideInPlay);
				Viz->PolygonVertices = Polygon2D;
				Viz->PolyZoneHeight = ZoneHeight;
				Viz->PolyColor = ZoneColor;

				if( GetWorld() && GetWorld()->IsGameWorld() )
				{
					Viz->RebuildVisualizer(); // We have to manually trigger the build, or it will not show in play
				}
			}
		}
	}
	#endif
}

TArray<AActor*> APolyZone::GetAllActorsWithinPolyZone()
{
	if( !ActorTracking )
	{
		DoActorTracking(); //If actor tracking is disabled, call it once so we have our list of actors
	}
	return ActorsInPolyZone;
}

void APolyZone::GetAllActorsOfClassWithinPolyZone(TSubclassOf<AActor> Class, TArray<AActor*>& Actors)
{
	if( !ActorTracking )
	{
		DoActorTracking(); //If actor tracking is disabled, call it once so we have our list of actors
	}

	if( Class )
	{
		for( AActor* Actor : ActorsInPolyZone )
		{
			if( Actor->IsA(Class) )
			{
				Actors.Add(Actor); // If we are the filtered subclass, add us to the output
			}
		}
	}
}

bool APolyZone::IsActorWithinPolyZone(AActor* Actor, bool SkipHeight)
{
	if( !IsValid(Actor) ) return false;
	return IsPointWithinPolyZone(Actor->GetActorLocation(), SkipHeight);
}

bool APolyZone::IsPointWithinPolyZone(FVector TestPoint, bool SkipHeight)
{
	// Height Check
	if( !SkipHeight && !FMath::IsWithinInclusive(TestPoint.Z, GridOrigin.Z, GridOrigin.Z + ZoneHeight) )
	{
		return false;
	}

	// 2D Bounds Check
	if( TestPoint.X < Bounds_MinX || TestPoint.X > Bounds_MaxX || TestPoint.Y < Bounds_MinY || TestPoint.Y > Bounds_MaxY )
	{
		return false;
	}

	// Grid check
	if( UsesGrid )
	{
		POLYZONE_CELL_FLAGS CellFlag = GetFlagAtLocation(TestPoint);
		if( CellFlag == POLYZONE_CELL_FLAGS::Outside ) return false;
		if( CellFlag == POLYZONE_CELL_FLAGS::Within ) return true;
	}

	return IsPointWithinPolygon(FVector2D(TestPoint.X, TestPoint.Y));
}

TArray<FVector> APolyZone::GetRandomPointsInPolyZone(int NumPoints, bool RandomHeight)
{
	TArray<FVector> RandomPoints;

	FVector MinBounds = PolyBounds.Origin - PolyBounds.BoxExtent;
	FVector MaxBounds = PolyBounds.Origin + PolyBounds.BoxExtent;

	int Failures = 0;
	int MaxFailures = FMath::Max(100.0f, NumPoints * 4);

	while( RandomPoints.Num() < NumPoints && Failures < MaxFailures )
	{
		float RandomX = FMath::FRandRange(MinBounds.X, MaxBounds.X);
		float RandomY = FMath::FRandRange(MinBounds.Y, MaxBounds.Y);
		float HeightToAdd = RandomHeight ? FMath::FRandRange(0.0f, ZoneHeight) : 0.0f;
		float RandomZ = GetActorLocation().Z + HeightToAdd;

		FVector RandomPoint(RandomX, RandomY, RandomZ);

		if( IsPointWithinPolygon(FVector2D(RandomX, RandomY)) )
		{
			RandomPoints.Add(RandomPoint);
		}
		else
		{
			Failures++;
		}
	}

	return RandomPoints;
}

TArray<FVector> APolyZone::GetRandomPointsAlongPolyZoneEdges(int NumPoints, bool RandomHeight)
{
	TArray<FVector> RandomPoints;
	if( IsValid(PolySpline) )
	{
		float SplineLength = PolySpline->GetSplineLength();

		for( int i = 0; i < NumPoints; ++i )
		{
			float HeightToAdd = RandomHeight ? FMath::FRandRange(0.0f, ZoneHeight) : 0.0f;
			float RandomDistance = FMath::FRandRange(0.0f, SplineLength);
			FVector DistanceAsLocation = PolySpline->GetWorldLocationAtDistanceAlongSpline(RandomDistance);
			DistanceAsLocation.Z = GetActorLocation().Z + HeightToAdd;
			RandomPoints.Add(DistanceAsLocation);
		}
	}

	return RandomPoints;
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
	int NumPoints = Polygon.Num();

	bool InsidePoly = false;
	for( int i = 0, j = NumPoints - 1; i < NumPoints; j = i++ )
	{
		FVector Point_i = Polygon[i];
		FVector Point_j = Polygon[j];
		if( (Point_i.Y > TestPoint.Y) != (Point_j.Y > TestPoint.Y) &&
			TestPoint.X < (Point_j.X - Point_i.X) * (TestPoint.Y - Point_i.Y) / (Point_j.Y - Point_i.Y) + Point_i.X )
		{
			InsidePoly = !InsidePoly;
		}
	}

	return InsidePoly;
}
// END MIT LICENSE

bool APolyZone::IsPointInAABB_2D(const FVector2D& Point, const FVector2D& Min, const FVector2D& Max)
{
	return Point.X >= Min.X && Point.X <= Max.X && Point.Y >= Min.Y && Point.Y <= Max.Y;
}

float APolyZone::Cross2D(const FVector2D& A, const FVector2D& B, const FVector2D& C)
{
	return (B.X - A.X) * (C.Y - A.Y) - (B.Y - A.Y) * (C.X - A.X);
}

bool APolyZone::IsOnSegment2D(const FVector2D& A, const FVector2D& B, const FVector2D& P)
{
	return P.X >= FMath::Min(A.X, B.X) && P.X <= FMath::Max(A.X, B.X) &&
		P.Y >= FMath::Min(A.Y, B.Y) && P.Y <= FMath::Max(A.Y, B.Y);
}

bool APolyZone::SegmentsIntersect2D(const FVector2D& A, const FVector2D& B, const FVector2D& C, const FVector2D& D)
{
	const float AB_C = Cross2D(A, B, C);
	const float AB_D = Cross2D(A, B, D);
	const float CD_A = Cross2D(C, D, A);
	const float CD_B = Cross2D(C, D, B);

	if( (AB_C == 0.0f && IsOnSegment2D(A, B, C)) || (AB_D == 0.0f && IsOnSegment2D(A, B, D)) ||
		(CD_A == 0.0f && IsOnSegment2D(C, D, A)) || (CD_B == 0.0f && IsOnSegment2D(C, D, B)) )
	{
		return true;
	}

	return (AB_C > 0.0f) != (AB_D > 0.0f) && (CD_A > 0.0f) != (CD_B > 0.0f);
}

FVector APolyZone::GetGridCellWorld(const FPolyZone_GridCell& Cell)
{
	return GridOrigin + FVector(Cell.X * CellSize, Cell.Y * CellSize, 0.0f);
}

FVector APolyZone::GetGridCellCenterWorld(const FPolyZone_GridCell& Cell)
{
	return GetGridCellWorld(Cell) + FVector(CellSize * 0.5f, CellSize * 0.5f, 0.0f);
}

FPolyZone_GridCell APolyZone::GetGridCellAtLocation(FVector Location)
{
	FTransform GridOriginTransform = FTransform(FVector(GridOrigin.X, GridOrigin.Y, 0.0f));
	FVector LocationOnGrid = GridOriginTransform.InverseTransformPosition(Location);
	int32 GridX = FMath::FloorToInt(LocationOnGrid.X / CellSize);
	int32 GridY = FMath::FloorToInt(LocationOnGrid.Y / CellSize);
	return FPolyZone_GridCell(GridX, GridY);
}

int32 APolyZone::GetGridCellIndex(const FPolyZone_GridCell& Cell) const
{
	if( Cell.X < 0 || Cell.Y < 0 || Cell.X >= GridCellsX || Cell.Y >= GridCellsY )
	{
		return INDEX_NONE;
	}

	return Cell.X + (Cell.Y * GridCellsX);
}

POLYZONE_CELL_FLAGS APolyZone::GetGridCellFlag(const FPolyZone_GridCell& Cell)
{
	const int32 CellIndex = GetGridCellIndex(Cell);
	if( CellIndex == INDEX_NONE || !GridData.IsValidIndex(CellIndex) )
	{
		return POLYZONE_CELL_FLAGS::Outside;
	}

	return GridData[CellIndex];
}

POLYZONE_CELL_FLAGS APolyZone::GetFlagAtLocation(FVector Location)
{
	return GetGridCellFlag(GetGridCellAtLocation(Location));
}

POLYZONE_CELL_FLAGS APolyZone::TestCellAgainstPolygon(FPolyZone_GridCell Cell)
{
	const FVector CellCenterLocation_WS = GetGridCellCenterWorld(Cell);
	const float HalfCellSize = CellSize * 0.5f;
	const FVector2D CellMin(CellCenterLocation_WS.X - HalfCellSize, CellCenterLocation_WS.Y - HalfCellSize);
	const FVector2D CellMax(CellCenterLocation_WS.X + HalfCellSize, CellCenterLocation_WS.Y + HalfCellSize);

	int Corner = -1;
	bool Result = false;
	bool OnPolyEdge = false;
	for( FVector2D CornerDir : CornerDirections )
	{
		FVector2D CellCorner;
		CellCorner.X = CellCenterLocation_WS.X + HalfCellSize * CornerDir.X;
		CellCorner.Y = CellCenterLocation_WS.Y + HalfCellSize * CornerDir.Y;

		bool CurResult = IsPointWithinPolygon(CellCorner);

		Corner++;
		if( Corner == 0 ) // First test
		{
			Result = CurResult;
		}
		else
		{
			if( CurResult != Result )
			{
				OnPolyEdge = true;
				break;
			}
		}
	}

	if( OnPolyEdge )
	{
		return POLYZONE_CELL_FLAGS::OnEdge;
	}
	if( Result )
	{
		return POLYZONE_CELL_FLAGS::Within;
	}

	// Any polygon vertex inside the cell?
	const int32 NumPoints = Polygon2D.Num();
	for( int32 i = 0; i < NumPoints; ++i )
	{
		if( IsPointInAABB_2D(Polygon2D[i], CellMin, CellMax) )
		{
			return POLYZONE_CELL_FLAGS::OnEdge;
		}
	}

	// Any polygon edge intersects any cell edge?
	const FVector2D CellTL(CellMin.X, CellMax.Y);
	const FVector2D CellTR(CellMax.X, CellMax.Y);
	const FVector2D CellBR(CellMax.X, CellMin.Y);
	const FVector2D CellBL(CellMin.X, CellMin.Y);

	for( int32 i = 0; i < NumPoints; ++i )
	{
		const FVector2D& A = Polygon2D[i];
		const FVector2D& B = Polygon2D[(i + 1) % NumPoints];

		if( SegmentsIntersect2D(A, B, CellTL, CellTR) || SegmentsIntersect2D(A, B, CellTR, CellBR) ||
			SegmentsIntersect2D(A, B, CellBR, CellBL) || SegmentsIntersect2D(A, B, CellBL, CellTL) )
		{
			return POLYZONE_CELL_FLAGS::OnEdge;
		}
	}

	return POLYZONE_CELL_FLAGS::Outside;
}

TArray<FPolyZone_GridCell> APolyZone::GetAllGridCells()
{
	TArray<FPolyZone_GridCell> Coords;
	if( GridCellsX <= 0 || GridCellsY <= 0 )
	{
		return Coords;
	}

	const int32 NumCells = GridData.Num();
	Coords.Reserve(NumCells);
	for( int32 Index = 0; Index < NumCells; ++Index )
	{
		if( GridData[Index] == POLYZONE_CELL_FLAGS::Outside )
		{
			continue;
		}

		const int32 GridX = Index % GridCellsX;
		const int32 GridY = Index / GridCellsX;
		Coords.Add(FPolyZone_GridCell(GridX, GridY));
	}
	return Coords;
}

void APolyZone::OnBeginBoundsOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
									 bool bFromSweep, const FHitResult& SweepResult)
{
	if( OtherActor->Implements<UPolyZone_Interface>() )
	{
		TrackedActors.Add(OtherActor, false);
	}
}

void APolyZone::OnEndBoundsOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if( OtherActor->Implements<UPolyZone_Interface>() )
	{
		if( TrackedActors.FindRef(OtherActor) ) // Get TMap Data: bool IsWithinPolygon
		{
			PolyZoneOverlapChange(OtherActor, false); // Notify that we left via bounds (likely height)
		}
		TrackedActors.Remove(OtherActor);
	}
}

// Track actors within bounds
void APolyZone::DoActorTracking()
{
	// We cannot notify while in the TMap loop, because the notifies may cause the map to change
	TArray<TPair<AActor*, bool>> ActorsToNotify;

	// Check if each tracked actor is within the polyzone
	for( const TPair<AActor*, bool>& MapPair : TrackedActors )
	{
		if( !IsValid(this) ) { break; }

		AActor* TrackedActor = MapPair.Key;
		bool IsWithinPoly = MapPair.Value;
		if( IsValid(TrackedActor) ) // TMap magically removes invalid actors but this is for my sanity
		{
			bool NewIsWithinPoly = IsActorWithinPolyZone(TrackedActor, true);
			if( NewIsWithinPoly != IsWithinPoly )
			{
				TrackedActors[TrackedActor] = NewIsWithinPoly;
				ActorsToNotify.Add(MapPair); // If our status has changed, we should notify the interface
			}
		}
	}

	if( ActorTracking ) // Only notify actors if actor tracking is enabled (this function might have been called from 'get' functions)
	{
		// Now that we know which actors have changed, we can notify their interfaces
		for( TPair<AActor*, bool>& ActorStatus : ActorsToNotify )
		{
			PolyZoneOverlapChange(ActorStatus.Key, ActorStatus.Value);
		}
	}
}

void APolyZone::PolyZoneOverlapChange(AActor* TrackedActor, bool NewIsOverlapped)
{
	if( NewIsOverlapped )
	{
		ActorsInPolyZone.Add(TrackedActor);
		OnEnterPolyZone(TrackedActor);
	}
	else
	{
		ActorsInPolyZone.Remove(TrackedActor);
		OnExitPolyZone(TrackedActor);
	}

	IPolyZone_Interface* ZoneInterface = Cast<IPolyZone_Interface>(TrackedActor);
	if( NewIsOverlapped )
	{
		ZoneInterface->Execute_EnterPolyZone(TrackedActor, this);
	}
	else
	{
		ZoneInterface->Execute_ExitPolyZone(TrackedActor, this);
	}
}

void APolyZone::DrawDebugGrid()
{
	if( !UsesGrid || GridCellsX <= 0 || GridCellsY <= 0 )
	{
		return;
	}

	UWorld* World = GetWorld();
	if( !World )
	{
		return;
	}

	const float HalfCellSize = CellSize * 0.5f;
	const float LineThickness = 1.0f;
	const float ShrunkHalfSize = FMath::Max(0.0f, HalfCellSize - (LineThickness * 0.5f));
	const FVector CellExtent(ShrunkHalfSize, ShrunkHalfSize, 5.0f);

	for( int32 GridX = 0; GridX < GridCellsX; GridX++ )
	{
		for( int32 GridY = 0; GridY < GridCellsY; GridY++ )
		{
			const FPolyZone_GridCell Cell(GridX, GridY);
			const POLYZONE_CELL_FLAGS CellFlag = GetGridCellFlag(Cell);

			FColor CellColor = FColor::White;
			if( CellFlag == POLYZONE_CELL_FLAGS::Within )
			{
				CellColor = FColor::Green;
			}
			else if( CellFlag == POLYZONE_CELL_FLAGS::OnEdge )
			{
				CellColor = FColor::Yellow;
			}

			FVector CellCenter = GetGridCellCenterWorld(Cell);
			CellCenter.Z = GridOrigin.Z;
			DrawDebugBox(World, CellCenter, CellExtent, CellColor, false, 0.0f, 0, LineThickness);
		}
	}
}
