// Copyright 2022 Seven47 Software. All Rights Reserved.

#include "PolyZone_Visualizer.h"
#include "GeometryScript/MeshPrimitiveFunctions.h"

APolyZone_Visualizer::APolyZone_Visualizer()
{
	DynamicMaterial = nullptr;
	bListedInSceneOutliner = false; // Hide from outliner
	PrimaryActorTick.bCanEverTick = false; // Tick

	PolyColor = FColor(0, 255, 0, 255);

	if( IsValid(DynamicMeshComponent) )
	{
		DynamicMeshComponent->SetShadowsEnabled(false);
	}
}

void APolyZone_Visualizer::BeginPlay()
{
	Super::BeginPlay();
	SetupDynamicMaterial(); // Sanity check
}

void APolyZone_Visualizer::SetupDynamicMaterial()
{
	if( IsValid(DynamicMeshComponent) )
	{
		FSoftObjectPath DefaultMaterialPath(TEXT("Material'/PolyZones_Plugin/PolyZoneDebugMaterial.PolyZoneDebugMaterial'"));
		UMaterial* DefaultMaterial = Cast<UMaterial>(DefaultMaterialPath.ResolveObject()); // Try getting already loaded material
		if( !IsValid(DefaultMaterial) )
		{
			DefaultMaterial = CastChecked<UMaterial>(DefaultMaterialPath.TryLoad()); // If not loaded already, load it
		}
		// Create a dynamic version so we can recolor it
		DynamicMaterial = UMaterialInstanceDynamic::Create(DefaultMaterial, this, FName("M_CreatedInstance"));
		DynamicMaterial->SetVectorParameterValue(FName(TEXT("Color")), FLinearColor(PolyColor));
		DynamicMeshComponent->SetMaterial(0, DynamicMaterial);
	}
}

// MESH FUNCTIONS

void APolyZone_Visualizer::ExecuteRebuildGeneratedMeshIfPending()
{
	if( bGeneratedMeshRebuildPending )
	{
		Super::ExecuteRebuildGeneratedMeshIfPending();
		RebuildMesh(DynamicMeshComponent->GetDynamicMesh());
	}
	else
	{
		Super::ExecuteRebuildGeneratedMeshIfPending();
	}
}

void APolyZone_Visualizer::RebuildVisualizer()
{
	RebuildMesh(DynamicMeshComponent->GetDynamicMesh());
}

void APolyZone_Visualizer::RebuildMesh(UDynamicMesh* TargetMesh)
{
	if( IsValid(TargetMesh) && PolygonVertices.Num() >= 3 )
	{
		FGeometryScriptPrimitiveOptions PrimitiveOptions;
		PrimitiveOptions.PolygroupMode = EGeometryScriptPrimitivePolygroupMode::PerFace;
		PrimitiveOptions.bFlipOrientation = true;
		PrimitiveOptions.UVMode = EGeometryScriptPrimitiveUVMode::Uniform;

		FTransform Transform;

		UGeometryScriptLibrary_MeshPrimitiveFunctions::AppendSimpleExtrudePolygon(TargetMesh, PrimitiveOptions, Transform, PolygonVertices, PolyZoneHeight, 0, false);

		SetupDynamicMaterial();
	}
}
