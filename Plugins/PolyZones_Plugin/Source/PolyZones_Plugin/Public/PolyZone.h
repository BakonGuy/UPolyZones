// Copyright 2022 Seven47 Software. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PolyZone_Grid.h"
#include "Components/SplineComponent.h"
#include "PolyZone.generated.h"

UCLASS( hidecategories = (Input), meta = (PrioritizeCategories = "PolyZone") )
class POLYZONES_PLUGIN_API APolyZone : public AActor
{
	GENERATED_BODY()

protected: // Accessible by subclasses
	virtual void OnConstruction(const FTransform& Transform) override; // Construction Script
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void K2_DestroyActor() override;
	bool WantsDestroyed = false; // A blueprint called destroy on us

	/*Called at the end of C++ construction*/
	UFUNCTION(BlueprintImplementableEvent, Category = "PolyZone")
	void PolyZoneConstructed();
	
public:	// Accessible anywhere
	APolyZone();

	// ~~ Overrides
	virtual void Tick(float DeltaTime) override;

	// ~~ Default actor components
	
	UPROPERTY()
	USplineComponent* PolySpline;

	UPROPERTY(Transient)
	UShapeComponent* BoundsOverlap;

	// ~~ PolyZone Config

	/*Track and notify actors that enter/exit the PolyZone
	 *With this disabled the PolyZone will not do anything on it's own, manual calls to the WithinPolyZone functions will be needed*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PolyZone Config")
	bool ActorTracking;
	
	/*Color associated with this zone, can be useful for showing debug text*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PolyZone Config")
	FColor ZoneColor;

	/*Object type to use as the PolyZone collision*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PolyZone Config")
	TEnumAsByte<ECollisionChannel> ZoneObjectType;

	/*Object types that will generate overlap events
	 *actors must have a collision of one of these types to be considered within the PolyZone*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PolyZone Config")
	TArray<TEnumAsByte<ECollisionChannel>> OverlapTypes;

	/*The height of the PolyZone's overlap bounds
	 *If you need an infinite height, you will need to call the WithinPolyZone functions manually with "SkipHeight" enabled*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PolyZone Config")
	float ZoneHeight;

#if WITH_EDITORONLY_DATA
	/*Draw the PolyZone walls while in editor*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PolyZone Config")
	bool ShowVisualization;

	/*Hides the visualization while in PIE (Note: The visualization does not exist in packaged builds, regardless of this setting)*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PolyZone Config")
	bool HideInPlay;
	
	UPROPERTY()
	UBillboardComponent* PolyIcon;

	UPROPERTY()
	UChildActorComponent* EditorVisualizer;
#endif
	
	// ~~ PolyZone I/O

	UFUNCTION(BlueprintImplementableEvent, Category = "PolyZone")
	void OnEnterPolyZone(AActor* EnteredActor);

	UFUNCTION(BlueprintImplementableEvent, Category = "PolyZone")
	void OnExitPolyZone(AActor* ExitedActor);
	
	UFUNCTION(BlueprintCallable, Category = "PolyZone")
	bool IsActorWithinPolyZone(AActor* Actor, bool SkipHeight = false, bool SkipBounds = false);
	
	UFUNCTION(BlueprintCallable, Category = "PolyZone")
	bool IsPointWithinPolyZone(FVector TestPoint, bool SkipHeight = false, bool SkipBounds = false);

	UFUNCTION(BlueprintCallable, Category = "PolyZone")
	TArray<AActor*> GetAllActorsWithinPolyZone();

	UFUNCTION(BlueprintCallable, Category = "PolyZone", meta=(DeterminesOutputType="Class", DynamicOutputParam="Actors"))
	void GetAllActorsOfClassWithinPolyZone(TSubclassOf<AActor> Class, TArray<AActor*>& Actors);

	// ~~ PolyZone Grid I/O
	
	UFUNCTION(/*BlueprintCallable, Category = "PolyZoneGrid"*/)
	TArray<FPolyZone_GridCell> GetAllGridCells();

	UFUNCTION(/*BlueprintCallable, Category = "PolyZoneGrid"*/)
	FPolyZone_GridCell GetGridCellAtLocation(FVector Location);

	UFUNCTION(/*BlueprintCallable, Category = "PolyZoneGrid"*/)
	FVector GetGridCellWorld(const FPolyZone_GridCell& Cell);

	UFUNCTION(/*BlueprintCallable, Category = "PolyZoneGrid"*/)
	FVector GetGridCellCenterWorld(const FPolyZone_GridCell& Cell);
	
	UFUNCTION(/*BlueprintCallable, Category = "PolyZoneGrid"*/)
	POLYZONE_CELL_FLAGS GetGridCellFlag(const FPolyZone_GridCell& Cell);

	UFUNCTION(/*BlueprintCallable, Category = "PolyZoneGrid"*/)
	POLYZONE_CELL_FLAGS GetFlagAtLocation(FVector Location);
	
	/*Origin of the PolyZone grid in world space (Also the location of Grid 0,0)*/
	UPROPERTY(/*BlueprintReadOnly, Category = "PolyZoneGrid"*/)
	FVector GridOrigin;

private: // Accessible by this class only
	void Build_PolyZone();
	void Construct_Polygon();
	void Construct_Bounds();
	void Construct_SetupGrid();
	void Construct_Visualizer();

	static int32 DivideNoRemainder(float Dividend, float Divisor)
	{
		int32 Result = 0;
		if (Divisor != 0.f)
		{
			const float Quotient = Dividend / Divisor;
			Result = (Quotient < 0.f ? -1 : 1) * FMath::FloorToInt(FMath::Abs(Quotient));
		}

		return Result;
	}

	// ~~ Bounds
	UPROPERTY()
	FBoxSphereBounds PolyBounds;
	UPROPERTY()
	double Bounds_MinX;
	UPROPERTY()
	double Bounds_MaxX;
	UPROPERTY()
	double Bounds_MinY;
	UPROPERTY()
	double Bounds_MaxY;
	
	UFUNCTION()
	void OnBeginBoundsOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnEndBoundsOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// ~~ Grid
	UPROPERTY()
	bool UsesGrid;
	UPROPERTY()
	float CellSize;
	UPROPERTY()
	int32 GridCellsX;
	UPROPERTY()
	int32 GridCellsY;

	UPROPERTY()
	TMap<FPolyZone_GridCell, POLYZONE_CELL_FLAGS> GridData;

	TArray<FVector2D> CornerDirections; // Multipliers to get each corner of a cell
	POLYZONE_CELL_FLAGS TestCellAgainstPolygon(FPolyZone_GridCell Cell);

	// ~~ Polygon
	bool IsPointWithinPolygon(FVector2D TestPoint);

	UPROPERTY()
	TArray<FVector> Polygon;

	UPROPERTY()
	TArray<FVector2D> Polygon2D;

	// ~~ Actor Tracking (Actors within box bounds)
	UPROPERTY()
	TMap<AActor*, bool> TrackedActors; // Map of all actors within the box bounds, and if they are within the PolyZone

	UPROPERTY()
	TArray<AActor*> ActorsInPolyZone; // All tracked actors currently within the PolyZone

	void DoActorTracking();
	void PolyZoneOverlapChange(AActor* TrackedActor, bool NewIsOverlapped);

};
