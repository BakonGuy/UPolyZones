// Copyright Seven47 Software All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GeometryActors/GeneratedDynamicMeshActor.h"
#include "PolyZone_Visualizer.generated.h"

UCLASS(Transient)
class APolyZone_Visualizer : public AGeneratedDynamicMeshActor
{
	GENERATED_BODY()

public:
	APolyZone_Visualizer(); // Constructor

private:
	virtual void PostInitProperties() override; // Runs in editor and game, after variables initialized

protected:
	virtual void ExecuteRebuildGeneratedMeshIfPending() override;
	virtual void BeginPlay() override;

	UPROPERTY()
	UMaterialInstanceDynamic* DynamicMaterial;
	
	void SetupDynamicMaterial();
	void RebuildMesh(UDynamicMesh* TargetMesh);

public:
	UPROPERTY()
	TArray<FVector2D> PolygonVertices;

	UPROPERTY()
	float PolyZoneHeight = 500.0f;
	
};
