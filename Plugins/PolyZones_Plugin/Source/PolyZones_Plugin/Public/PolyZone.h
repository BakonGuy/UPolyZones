// Copyright 2022-2026 Overtorque Creations LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PolyZone_Grid.h"
#include "Components/SplineComponent.h"
// ReSharper disable once CppUnusedIncludeDirective
#include "Components/ShapeComponent.h" // Needed for compiling in Game Mode
#include "PolyZone.generated.h"

UCLASS(HideCategories=(Input), meta=(PrioritizeCategories="PolyZone"))
class POLYZONES_PLUGIN_API APolyZone : public AActor
{
	GENERATED_BODY()

public:
	APolyZone();

	// ==================== ENGINE OVERRIDES ====================
	virtual void OnConstruction(const FTransform& Transform) override; // Construction Script
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void K2_DestroyActor() override;

	// ==================== INPUTS & OUTPUTS ====================

	UFUNCTION(BlueprintImplementableEvent, Category = "PolyZone")
	void OnEnterPolyZone(AActor* EnteredActor);

	UFUNCTION(BlueprintImplementableEvent, Category = "PolyZone")
	void OnExitPolyZone(AActor* ExitedActor);

	UFUNCTION(BlueprintCallable, Category = "PolyZone")
	bool IsActorWithinPolyZone(AActor* Actor, bool SkipHeight = false, bool SkipBounds = false);

	UFUNCTION(BlueprintCallable, Category = "PolyZone")
	TArray<AActor*> GetAllActorsWithinPolyZone();

	UFUNCTION(BlueprintCallable, Category = "PolyZone")
	bool IsPointWithinPolyZone(FVector TestPoint, bool SkipHeight = false, bool SkipBounds = false);

	UFUNCTION(BlueprintCallable, Category = "PolyZone")
	TArray<FVector> GetRandomPointsInPolyZone(int NumPoints, bool RandomHeight);

	UFUNCTION(BlueprintCallable, Category = "PolyZone")
	TArray<FVector> GetRandomPointsAlongPolyZoneEdges(int NumPoints, bool RandomHeight);

	UFUNCTION(BlueprintCallable, Category = "PolyZone", meta=(DeterminesOutputType="Class", DynamicOutputParam="Actors"))
	void GetAllActorsOfClassWithinPolyZone(TSubclassOf<AActor> Class, TArray<AActor*>& Actors);

	UFUNCTION(BlueprintCallable, Category = "PolyZone|Grid")
	TArray<FPolyZone_GridCell> GetAllGridCells();

	UFUNCTION(BlueprintCallable, Category = "PolyZone|Grid")
	FPolyZone_GridCell GetGridCellAtLocation(FVector Location);

	UFUNCTION(BlueprintCallable, Category = "PolyZone|Grid")
	FVector GetGridCellWorld(const FPolyZone_GridCell& Cell);

	UFUNCTION(BlueprintCallable, Category = "PolyZone|Grid")
	FVector GetGridCellCenterWorld(const FPolyZone_GridCell& Cell);

	UFUNCTION(BlueprintCallable, Category = "PolyZone|Grid")
	POLYZONE_CELL_FLAGS GetGridCellFlag(const FPolyZone_GridCell& Cell);

	UFUNCTION(BlueprintCallable, Category = "PolyZone|Grid")
	POLYZONE_CELL_FLAGS GetFlagAtLocation(FVector Location);

protected:
	
	/*Called at the end of C++ construction*/
	UFUNCTION(BlueprintImplementableEvent, Category = "PolyZone")
	void PolyZoneConstructed();

	// ==================== CONFIGURATION ====================
public:
	// -- Default actor components --

	UPROPERTY(BlueprintReadOnly, Category = "PolyZone")
	USplineComponent* PolySpline = nullptr;

	UPROPERTY(Transient)
	UShapeComponent* BoundsOverlap = nullptr;

	// -- PolyZone Config --

	/*Track and notify actors that enter/exit the PolyZone
	 *With this disabled the PolyZone will not do anything on it's own, manual calls to the WithinPolyZone functions will be needed*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PolyZone Config")
	bool ActorTracking = true;

	/*Color associated with this zone, can be useful for showing debug text*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PolyZone Config")
	FColor ZoneColor = FColor(0, 255, 0, 255);

	/*Object type to use as the PolyZone collision*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PolyZone Config")
	TEnumAsByte<ECollisionChannel> ZoneObjectType = ECollisionChannel::ECC_WorldDynamic;

	/*Object types that will generate overlap events
	 *actors must have a collision of one of these types to be considered within the PolyZone*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PolyZone Config")
	TArray<TEnumAsByte<ECollisionChannel>> OverlapTypes;

	/*The height of the PolyZone's overlap bounds
	 *If you need an infinite height, you will need to call the WithinPolyZone functions manually with "SkipHeight" enabled*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PolyZone Config")
	float ZoneHeight = 250.0f;

	/*Draw grid cell debug boxes in the world*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PolyZone Config", AdvancedDisplay)
	bool bDebugGrid = false;
	
	#if WITH_EDITORONLY_DATA
	/*Draw the PolyZone walls while in editor*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PolyZone Config")
	bool ShowVisualization = true;

	/*Hides the visualization while playing in editor (Note: The visualization is editor only and does not exist in packaged builds, regardless of this setting)*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PolyZone Config")
	bool HideInPlay = true;

	UPROPERTY()
	UBillboardComponent* PolyIcon = nullptr;

	UPROPERTY()
	UChildActorComponent* EditorVisualizer = nullptr;
	#endif

	// ==================== FUNCTIONALITY ====================
public:
	// -- PolyZone Grid --

	/*Origin of the PolyZone grid in world space (Also the location of Grid 0,0)*/
	UPROPERTY(BlueprintReadOnly, Category = "PolyZone|Grid")
	FVector GridOrigin = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "PolyZone|Grid")
	TMap<FPolyZone_GridCell, POLYZONE_CELL_FLAGS> GridData = {};

	UPROPERTY(BlueprintReadOnly, Category = "PolyZone|Grid")
	float CellSize = 50.0f;
	
private:
	void Build_PolyZone();
	void Construct_Polygon();
	void Construct_Bounds();
	void Construct_SetupGrid();
	void Construct_Visualizer();
	void DoActorTracking();
	void PolyZoneOverlapChange(AActor* TrackedActor, bool NewIsOverlapped);
	bool IsPointWithinPolygon(FVector2D TestPoint);
	static bool IsPointInAABB_2D(const FVector2D& Point, const FVector2D& Min, const FVector2D& Max);
	static float Cross2D(const FVector2D& A, const FVector2D& B, const FVector2D& C);
	static bool IsOnSegment2D(const FVector2D& A, const FVector2D& B, const FVector2D& P);
	static bool SegmentsIntersect2D(const FVector2D& A, const FVector2D& B, const FVector2D& C, const FVector2D& D);
	POLYZONE_CELL_FLAGS TestCellAgainstPolygon(FPolyZone_GridCell Cell);
	void DrawDebugGrid();
	
	bool WantsDestroyed = false; // A blueprint called destroy on us
	
	// -- Bounds --
	FBoxSphereBounds PolyBounds = FBoxSphereBounds();
	double Bounds_MinX = 0.0;
	double Bounds_MaxX = 0.0;
	double Bounds_MinY = 0.0;
	double Bounds_MaxY = 0.0;

	// -- Grid --
	bool UsesGrid = false;
	int32 GridCellsX = 0;
	int32 GridCellsY = 0;

	TArray<FVector2D> CornerDirections; // Multipliers to get each corner of a cell

	// -- Polygon --
	TArray<FVector> Polygon;
	TArray<FVector2D> Polygon2D;

	// -- Actor Tracking (Actors within box bounds) --
	UPROPERTY()
	TMap<AActor*, bool> TrackedActors = {}; // Map of all actors within the box bounds, and if they are within the PolyZone

	UPROPERTY()
	TArray<AActor*> ActorsInPolyZone; // All tracked actors currently within the PolyZone

	UFUNCTION()
	void OnBeginBoundsOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnEndBoundsOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
