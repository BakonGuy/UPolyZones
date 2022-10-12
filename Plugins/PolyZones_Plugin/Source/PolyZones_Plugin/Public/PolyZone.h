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
	float CellSize;

	// Poly Zone functions

	UFUNCTION(BlueprintCallable, Category = "Zones")
	bool IsPointWithinPolygon(FVector2D TestPoint);

	UFUNCTION(BlueprintCallable, Category = "Zones")
	TArray<FPolyZone_GridCell> GetAllGridCells();

	UFUNCTION(BlueprintCallable, Category = "Zones")
	FVector2D GetGridCellWorld(const FPolyZone_GridCell& Cell);

	UFUNCTION(BlueprintCallable, Category = "Zones")
	FVector2D GetGridCellCenterWorld(const FPolyZone_GridCell& Cell);

	UFUNCTION(BlueprintCallable, Category = "Zones")
	POLYZONE_CELL_FLAGS GetGridCellFlag(const FPolyZone_GridCell& Cell);

protected:
	virtual void OnConstruction(const FTransform& Transform) override; // Construction Script
	virtual void BeginPlay() override;

	void Construct_Spline();
	void Construct_SetupGrid();

public:	
	virtual void Tick(float DeltaTime) override;

	// Make private later

	// Grid's origin in world space
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zones")
	FVector2D GridOrigin_WS;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zones")
	int CellsX;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zones")
	int CellsY;

private:
	TMap<FPolyZone_GridCell, POLYZONE_CELL_FLAGS> GridData;

	TArray<FVector2D> CornerDirections; // Multipliers to get each corner of a cell
	POLYZONE_CELL_FLAGS TestCellAgainstPolygon(FPolyZone_GridCell Cell);

};
