// Copyright Seven47 Software All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PolyZone_Grid.h"
#include "Components/SplineComponent.h"
#include "PolyZone.generated.h"

UCLASS( hidecategories = (Input), meta = (PrioritizeCategories = "PolyZone") )
class POLYZONES_PLUGIN_API APolyZone : public AActor
{
	GENERATED_BODY()
	
public:	
	APolyZone();

	// Default actor components
	
	UPROPERTY()
	USplineComponent* PolySpline;

	UPROPERTY(Transient)
	UShapeComponent* BoundsOverlap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PolyZone")
	FColor ZoneColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PolyZone")
	TEnumAsByte<ECollisionChannel> ZoneObjectType;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PolyZone")
	TArray<TEnumAsByte<ECollisionChannel>> OverlapTypes;

#if WITH_EDITORONLY_DATA // Editor only variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PolyZone")
	bool ShowVisualization;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PolyZone")
	bool HideInPlay;
	
	UPROPERTY()
	UBillboardComponent* PolyIcon;

	UPROPERTY()
	UChildActorComponent* EditorVisualizer;
#endif

	// Configuration Options
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PolyZone")
	float ZoneHeight;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PolyZone")
	float CellSize;

	// Poly Zone functions

	UFUNCTION(BlueprintCallable, Category = "PolyZone")
	bool IsPointWithinPolyZone(FVector TestPoint, bool InfiniteHeight);

	UFUNCTION(BlueprintCallable, Category = "PolyZone")
	bool IsTrackedActorWithinPolyZone(AActor* TrackedActor);
	
	UFUNCTION(BlueprintCallable, Category = "PolyZone")
	bool IsPointWithinPolygon(FVector2D TestPoint);

	UFUNCTION(BlueprintCallable, Category = "PolyZone")
	TArray<FPolyZone_GridCell> GetAllGridCells();

	UFUNCTION(BlueprintCallable, Category = "PolyZone")
	FVector GetGridCellWorld(const FPolyZone_GridCell& Cell);

	UFUNCTION(BlueprintCallable, Category = "PolyZone")
	FVector GetGridCellCenterWorld(const FPolyZone_GridCell& Cell);

	UFUNCTION(BlueprintCallable, Category = "PolyZone")
	FPolyZone_GridCell GetGridCellAtLocation(FVector Location);
	
	UFUNCTION(BlueprintCallable, Category = "PolyZone")
	POLYZONE_CELL_FLAGS GetGridCellFlag(const FPolyZone_GridCell& Cell);

	UFUNCTION(BlueprintCallable, Category = "PolyZone")
	POLYZONE_CELL_FLAGS GetFlagAtLocation(FVector Location);

	UFUNCTION(BlueprintCallable, Category = "PolyZone")
	TArray<AActor*> GetAllActorsInPolyZone() { return ActorsInPolyZone; }

protected:
	virtual void OnConstruction(const FTransform& Transform) override; // Construction Script
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void K2_DestroyActor() override;
	bool WantsDestroyed = false; // A blueprint called destroy on us

public:	
	virtual void Tick(float DeltaTime) override;

	// Make private later

	// Grid's origin in world space
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PolyZone")
	FVector GridOrigin_WS;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PolyZone")
	int CellsX;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PolyZone")
	int CellsY;

	UFUNCTION(BlueprintImplementableEvent, Category = "PolyZone")
	void PolyZoneConstructed();

	// Bounds Overlaps
	
	UFUNCTION()
	void OnBeginBoundsOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnEndBoundsOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintImplementableEvent)
	void OnEnterPolyZone(AActor* EnteredActor);

	UFUNCTION(BlueprintImplementableEvent)
	void OnExitPolyZone(AActor* ExitedActor);

private:
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
	
	UPROPERTY()
	TMap<FPolyZone_GridCell, POLYZONE_CELL_FLAGS> GridData;

	UPROPERTY()
	TArray<FVector> Polygon;

	UPROPERTY()
	TArray<FVector2D> Polygon2D;

	// Bounds
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

	TArray<FVector2D> CornerDirections; // Multipliers to get each corner of a cell
	POLYZONE_CELL_FLAGS TestCellAgainstPolygon(FPolyZone_GridCell Cell);

	// Actor Tracking
	UPROPERTY()
	TMap<AActor*, bool> TrackedActors;

	UPROPERTY()
	TArray<AActor*> ActorsInPolyZone;

	void ZoneOverlapChange(AActor* TrackedActor, bool NewIsOverlapped);

};
