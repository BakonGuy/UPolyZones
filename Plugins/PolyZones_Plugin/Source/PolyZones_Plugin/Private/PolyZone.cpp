// Copyright Seven47 Software All Rights Reserved.

#include "PolyZone.h"

#include "PolyZone_Interface.h"
#include "PolyZone_Visualizer.h"
#include "Components/BillboardComponent.h"
#include "Components/BoxComponent.h"
#include "GeometryFramework/Public/Components/DynamicMeshComponent.h"

// Sets default values
APolyZone::APolyZone()
{
	PrimaryActorTick.bCanEverTick = true; // Tick

	// Defaults
	ZoneColor = FColor(0, 255, 0, 255);
	
	ZoneObjectType = ECollisionChannel::ECC_WorldDynamic;
	OverlapTypes.Add(ECollisionChannel::ECC_Pawn);
	
	ZoneHeight = 500.0f;
	CellSize = 50.0f;
	CellsX = 0;
	CellsY = 0;

	CornerDirections.Add( FVector2D(-1.0f, -1.0f) ); // BL
	CornerDirections.Add( FVector2D(1.0f, -1.0f) ); // TL
	CornerDirections.Add( FVector2D(1.0f, 1.0f) ); // TR
	CornerDirections.Add( FVector2D(-1.0f, 1.0f) ); // BR

	// Initializations
	USceneComponent* SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComp"));
	SetRootComponent(SceneComponent); // Set actor root before creating other components
	RootComponent->Mobility = EComponentMobility::Static;

	PolySpline = CreateDefaultSubobject<USplineComponent>("PolySpline");
	PolySpline->SetupAttachment(RootComponent);

#if WITH_EDITORONLY_DATA // Editor only defaults
	bRunConstructionScriptOnDrag = false; // Allow spline editing without the lag
	ShowVisualization = true;

	PolyIcon = CreateEditorOnlyDefaultSubobject<UBillboardComponent>("PolyIcon");
	if(PolyIcon)
	{
		PolyIcon->SetRelativeLocation(FVector(0.0f,0.0f,50.0f));
		PolyIcon->bUseAttachParentBound = true;
		PolyIcon->SetupAttachment(RootComponent);
	}
	PolyZoneVisualizer = CreateDefaultSubobject<UChildActorComponent>("PolyZoneVisualizer", true); // EditorSubObject hides the component in game view
	if(PolyZoneVisualizer)
	{
		PolyZoneVisualizer->SetChildActorClass(APolyZone_Visualizer::StaticClass());
		PolyZoneVisualizer->SetupAttachment(RootComponent); // TODO: Save and apply Polygon in local space so this can be attached
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

// Rebuilds the PolyZone (can be run during runtime)
void APolyZone::Build_PolyZone()
{
	if(PolySpline->GetNumberOfSplinePoints() < 3) // A polygon must have at least 3 points to be valid
	{
		// Create a default polygon
		TArray<FVector> DefaultPoints;
		DefaultPoints.Add(FVector(0, 0, 0));
		DefaultPoints.Add(FVector(100, -50, 0));
		DefaultPoints.Add(FVector(100, 50, 0));
		PolySpline->SetSplinePoints(DefaultPoints, ESplineCoordinateSpace::Local, true);
	}
	if(PolySpline->GetNumberOfSplinePoints() >= 3) // We check again because sometimes the default is overridden
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
	for(int i = 0; i <= LastSplineIndex; i++)
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
	for ( int i = 1 ; i < LastSplineIndex+1 ; i++ )
	{
		FVector q = Polygon[i];
		Bounds_MinX = FMath::Min( q.X, Bounds_MinX );
		Bounds_MaxX = FMath::Max( q.X, Bounds_MaxX );
		Bounds_MinY = FMath::Min( q.Y, Bounds_MinY );
		Bounds_MaxY = FMath::Max( q.Y, Bounds_MaxY );
	}
}

void APolyZone::Construct_Bounds()
{
	// TODO: Calculate smallest rectangular bounds for overlap
	PolyBounds = PolySpline->CalcBounds(PolySpline->GetComponentTransform());
	UBoxComponent* NewBoundsOverlap = NewObject<UBoxComponent>(this);
	NewBoundsOverlap->CreationMethod = EComponentCreationMethod::UserConstructionScript;
	if(NewBoundsOverlap)
	{
		// Create
		FVector OverlapExtent = FVector(PolyBounds.BoxExtent.X, PolyBounds.BoxExtent.Y, ZoneHeight*0.5f);
		NewBoundsOverlap->RegisterComponent();
		NewBoundsOverlap->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

#if WITH_EDITORONLY_DATA
		// Setup Visualization
		NewBoundsOverlap->ShapeColor = FColor(0, 255, 0);
		NewBoundsOverlap->SetVisibility( ShowVisualization );
		NewBoundsOverlap->SetHiddenInGame( false );
#endif

		// Setup Shape
		NewBoundsOverlap->SetBoxExtent(OverlapExtent);
		NewBoundsOverlap->SetWorldLocation(PolyBounds.Origin + FVector(0,0,ZoneHeight*0.5f));

		// Setup Collision
		NewBoundsOverlap->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		NewBoundsOverlap->SetCollisionObjectType(ZoneObjectType);
		NewBoundsOverlap->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		for(ECollisionChannel ToOverlap : OverlapTypes)
		{
			NewBoundsOverlap->SetCollisionResponseToChannel(ToOverlap, ECollisionResponse::ECR_Overlap);
		}
	}
	BoundsOverlap = NewBoundsOverlap;
}

void APolyZone::Construct_SetupGrid()
{
	GridData.Empty(); // Can rebuild at runtime
	
	CellsX = DivideNoRemainder( (PolyBounds.BoxExtent.X * 2.0f), CellSize ) + 1;
	CellsY = DivideNoRemainder( (PolyBounds.BoxExtent.Y * 2.0f), CellSize ) + 1;

	double OriginX = PolyBounds.Origin.X - (CellsX * 0.5f * CellSize);
	double OriginY = PolyBounds.Origin.Y - (CellsY * 0.5f * CellSize);
	GridOrigin_WS = FVector(OriginX, OriginY, GetActorLocation().Z);

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

void APolyZone::Construct_Visualizer()
{
#if WITH_EDITORONLY_DATA
	if(ShowVisualization)
	{
		if(IsValid(PolyZoneVisualizer))
		{
			PolyZoneVisualizer->SetWorldTransform(FTransform(FVector(0,0,GetActorLocation().Z))); // Move XY to world origin
			PolyZoneVisualizer->CreateChildActor();
			AActor* VizActor = PolyZoneVisualizer->GetChildActor();
			APolyZone_Visualizer* Viz = Cast<APolyZone_Visualizer>(VizActor);
			if(IsValid(Viz))
			{
				Viz->PolygonVertices = Polygon2D;
				Viz->PolyZoneHeight = ZoneHeight;
				Viz->PolyColor = ZoneColor;
			}
		}
	}
#endif
}

bool APolyZone::IsPointWithinPolyZone(FVector TestPoint, bool InfiniteHeight)
{
	// Height Check
	if(!InfiniteHeight && !FMath::IsWithinInclusive(TestPoint.Z, GridOrigin_WS.Z, GridOrigin_WS.Z + ZoneHeight))
	{
		return false;
	}

	// 2D Bounds Check
	if ( TestPoint.X < Bounds_MinX || TestPoint.X > Bounds_MaxX || TestPoint.Y < Bounds_MinY || TestPoint.Y > Bounds_MaxY )
	{
		return false;
	}

	// Grid check
	POLYZONE_CELL_FLAGS CellFlag = GetFlagAtLocation(TestPoint);
	if(CellFlag == POLYZONE_CELL_FLAGS::Within)
	{
		return true;
	}
	if(CellFlag == POLYZONE_CELL_FLAGS::OnEdge)
	{
		return IsPointWithinPolygon( FVector2D(TestPoint.X, TestPoint.Y) );
	}
	
	return false;
}

bool APolyZone::IsTrackedActorWithinPolyZone(AActor* TrackedActor)
{
	FVector TestPoint = TrackedActor->GetActorLocation();
	
	// Height Check
	if( !FMath::IsWithinInclusive(TestPoint.Z, GridOrigin_WS.Z, GridOrigin_WS.Z + ZoneHeight))
	{
		return false;
	}

	return IsPointWithinPolygon( FVector2D(TestPoint.X, TestPoint.Y) );
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
	for ( int i = 0, j = NumPoints - 1 ; i < NumPoints ; j = i++ )
	{
		FVector Point_i = Polygon[i];
		FVector Point_j = Polygon[j];
		if ( ( Point_i.Y > TestPoint.Y ) != ( Point_j.Y > TestPoint.Y ) &&
			 TestPoint.X < ( Point_j.X - Point_i.X ) * ( TestPoint.Y - Point_i.Y ) / ( Point_j.Y - Point_i.Y ) + Point_i.X )
		{
			InsidePoly = !InsidePoly;
		}
	}

	return InsidePoly;
}

FVector APolyZone::GetGridCellWorld(const FPolyZone_GridCell& Cell)
{
	return GridOrigin_WS + FVector(Cell.X * CellSize, Cell.Y * CellSize, 0.0f);
}

FVector APolyZone::GetGridCellCenterWorld(const FPolyZone_GridCell& Cell)
{
	return GetGridCellWorld(Cell) + FVector(CellSize*0.5f, CellSize*0.5f, 0.0f);
}

FPolyZone_GridCell APolyZone::GetGridCellAtLocation(FVector Location)
{
	FTransform GridOriginTransform = FTransform( FVector(GridOrigin_WS.X, GridOrigin_WS.Y, 0.0f) );
	FVector LocationLocalToGrid = GridOriginTransform.InverseTransformPosition(Location);
	int32 GridX = FMath::RoundToInt32(LocationLocalToGrid.X / CellSize);
	int32 GridY = FMath::RoundToInt32(LocationLocalToGrid.Y / CellSize);
	return FPolyZone_GridCell(GridX, GridY);
}

POLYZONE_CELL_FLAGS APolyZone::GetGridCellFlag(const FPolyZone_GridCell& Cell)
{
	return GridData.FindRef(Cell);
}

POLYZONE_CELL_FLAGS APolyZone::GetFlagAtLocation(FVector Location)
{
	return GetGridCellFlag(GetGridCellAtLocation(Location));
}

POLYZONE_CELL_FLAGS APolyZone::TestCellAgainstPolygon(FPolyZone_GridCell Cell)
{
	// Current implementation is an estimate, only tests if each corner is within polygon
	// If all corners are not the same (Within/Outside the polygon) then it's considered on the edge
	// TODO: Implement edge intersection testing
	FVector CellCenterLocation_WS = GetGridCellCenterWorld(Cell);

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
	Construct_Bounds();
	Construct_Visualizer();
	if( IsValid(BoundsOverlap) )
	{
		// Bind Overlap Events
		BoundsOverlap->OnComponentBeginOverlap.AddDynamic(this, &APolyZone::OnBeginBoundsOverlap);
		BoundsOverlap->OnComponentEndOverlap.AddDynamic(this, &APolyZone::OnEndBoundsOverlap);

		// Track pre-spawned actors
		TArray<AActor*> StartingOverlaps;
		BoundsOverlap->GetOverlappingActors(StartingOverlaps);
		int LastIndex = StartingOverlaps.Num()-1;
		for (int Index = 0; Index <= LastIndex; ++Index)
		{
			if( StartingOverlaps[Index]->Implements<UPolyZone_Interface>() )
			{
				TrackedActors.Add(StartingOverlaps[Index], false);
			}
		}
	}
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
			NotifyActorOfOverlapChange(OtherActor, false); // Notify that we left via bounds (likely height)
		}
		TrackedActors.Remove(OtherActor);
	}
}

// Called every frame
void APolyZone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	for (const TPair<AActor*, bool>& MapPair : TrackedActors)
	{
		AActor* TrackedActor = MapPair.Key;
		bool IsWithinPoly = MapPair.Value;
		if ( IsValid(TrackedActor) ) // TMap magically removes invalid actors but this is for my sanity
		{
			bool NewIsWithinPoly = IsTrackedActorWithinPolyZone( TrackedActor );
			if(NewIsWithinPoly != IsWithinPoly)
			{
				TrackedActors[TrackedActor] = NewIsWithinPoly;
				NotifyActorOfOverlapChange(TrackedActor, NewIsWithinPoly);
			}
		}
	}
}

void APolyZone::NotifyActorOfOverlapChange(AActor* TrackedActor, bool NewIsOverlapped)
{
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