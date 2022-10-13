// Copyright Seven47 Software All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PolyZone_Grid.h"
#include "Components/SplineComponent.h"
#include "GameFramework/Actor.h"
#include "PolyZone.generated.h"

UCLASS()
class APolyZone : public AActor
{
	GENERATED_BODY()
	
public:	
	APolyZone();

	// Default actor components
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zones")
	USplineComponent* PolySpline;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zones")
	UBillboardComponent* PolyIcon;

	// Configuration Options

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zones")
	float ZoneHeight;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zones")
	float CellSize;

	// Poly Zone functions

	UFUNCTION(BlueprintCallable, Category = "Zones")
	bool IsPointWithinPolyZone(FVector TestPoint);
	
	UFUNCTION(BlueprintCallable, Category = "Zones")
	bool IsPointWithinPolygon(FVector2D TestPoint);

	UFUNCTION(BlueprintCallable, Category = "Zones")
	TArray<FPolyZone_GridCell> GetAllGridCells();

	UFUNCTION(BlueprintCallable, Category = "Zones")
	FVector GetGridCellWorld(const FPolyZone_GridCell& Cell);

	UFUNCTION(BlueprintCallable, Category = "Zones")
	FVector GetGridCellCenterWorld(const FPolyZone_GridCell& Cell);

	UFUNCTION(BlueprintCallable, Category = "Zones")
	FPolyZone_GridCell GetGridCellAtLocation(FVector Location);
	
	UFUNCTION(BlueprintCallable, Category = "Zones")
	POLYZONE_CELL_FLAGS GetGridCellFlag(const FPolyZone_GridCell& Cell);

	UFUNCTION(BlueprintCallable, Category = "Zones")
	POLYZONE_CELL_FLAGS GetFlagAtLocation(FVector Location);

protected:
	virtual void OnConstruction(const FTransform& Transform) override; // Construction Script
	virtual void BeginPlay() override;

	void Construct_Spline();
	void Construct_SetupGrid();

public:	
	virtual void Tick(float DeltaTime) override;

	// Make private later

	// Grid's origin in world space
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zones")
	FVector GridOrigin_WS;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zones")
	int CellsX;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zones")
	int CellsY;

	UFUNCTION(BlueprintImplementableEvent, Category = "VehicleSystemPlugin")
	void PolyZoneConstructed();

private:
	UPROPERTY()
	TMap<FPolyZone_GridCell, POLYZONE_CELL_FLAGS> GridData;

	// Bounds
	UPROPERTY()
	FBoxSphereBounds PolyBounds;
	UPROPERTY()
	double minX;
	UPROPERTY()
	double maxX;
	UPROPERTY()
	double minY;
	UPROPERTY()
	double maxY;

	TArray<FVector2D> CornerDirections; // Multipliers to get each corner of a cell
	POLYZONE_CELL_FLAGS TestCellAgainstPolygon(FPolyZone_GridCell Cell);

};
