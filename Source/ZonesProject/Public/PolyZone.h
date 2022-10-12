// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "GameFramework/Actor.h"
#include "ZonesProject/Public/FPZ_GridCell.h"
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
	TArray<FPZ_GridCellCoord> GetAllGridCells();

	UFUNCTION(BlueprintCallable, Category = "Zones")
	FVector2D GetGridCellWorld(const FPZ_GridCellCoord& Cell);

	UFUNCTION(BlueprintCallable, Category = "Zones")
	FVector2D GetGridCellCenterWorld(const FPZ_GridCellCoord& Cell);

	UFUNCTION(BlueprintCallable, Category = "Zones")
	PZ_LOOKUP_CELL_FLAGS GetGridCellFlag(const FPZ_GridCellCoord& Cell);

protected:
	virtual void OnConstruction(const FTransform& Transform) override; // Construction Script
	virtual void BeginPlay() override;

	void Construct_Spline();
	void Construct_SetupGrid();

public:	
	virtual void Tick(float DeltaTime) override;

	// Make private later

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zones")
	FVector2D GridOrigin_WS;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zones")
	int CellsX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zones")
	int CellsY;

private:
	TMap<FPZ_GridCellCoord, PZ_LOOKUP_CELL_FLAGS> GridData;

	TArray<FVector2D> CornerDirections; // Multipliers to get each corner of a cell
	PZ_LOOKUP_CELL_FLAGS TestCellAgainstPolygon(FPZ_GridCellCoord Cell);

};
