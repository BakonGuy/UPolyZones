// Copyright Seven47 Software All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GeometryActors/GeneratedDynamicMeshActor.h"
#include "PolyZone_Visualizer.generated.h"

UCLASS(Transient)
class POLYZONES_EDITOR_API APolyZone_Visualizer : public AGeneratedDynamicMeshActor
{
	GENERATED_BODY()

public:
	APolyZone_Visualizer(); // Constructor

protected:
	virtual void ExecuteRebuildGeneratedMeshIfPending() override;
	virtual void BeginPlay() override;

	virtual bool IsEditorOnly() const override
	{
		return true;
	}

	UPROPERTY()
	UMaterialInstanceDynamic* DynamicMaterial;
	
	void SetupDynamicMaterial();
	void RebuildMesh(UDynamicMesh* TargetMesh);

public:
	UPROPERTY()
	TArray<FVector2D> PolygonVertices;

	UPROPERTY()
	float PolyZoneHeight = 500.0f;

	UPROPERTY()
	FColor PolyColor;
	
};
