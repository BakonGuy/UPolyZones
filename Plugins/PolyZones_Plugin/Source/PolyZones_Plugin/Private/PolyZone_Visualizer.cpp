// Copyright Seven47 Software All Rights Reserved.

#include "PolyZone_Visualizer.h"
#include "GeometryScript/MeshPrimitiveFunctions.h"

void APolyZone_Visualizer::PostInitProperties()
{
	Super::PostInitProperties();
	if(IsValid(DynamicMeshComponent))
	{
		FSoftObjectPath DefaultMaterialPath(TEXT("Material'/PolyZones_Plugin/PolyZoneDebugMaterial.PolyZoneDebugMaterial'"));
		UMaterial* DefaultMaterial = Cast<UMaterial>(DefaultMaterialPath.ResolveObject()); // Try getting already loaded material
		if ( !IsValid(DefaultMaterial) )
		{
			DefaultMaterial = CastChecked<UMaterial>(DefaultMaterialPath.TryLoad()); // If not loaded already, load it
		}
		// Create a dynamic version so we can recolor it
		UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(DefaultMaterial, this,FName("M_CreatedInstance"));
		DynamicMeshComponent->SetMaterial(0, DynamicMaterial);
	}
}

void APolyZone_Visualizer::ExecuteRebuildGeneratedMeshIfPending()
{
	if (bGeneratedMeshRebuildPending)
	{
		Super::ExecuteRebuildGeneratedMeshIfPending();
		RebuildMesh(DynamicMeshComponent->GetDynamicMesh());
	}
	else
	{
		Super::ExecuteRebuildGeneratedMeshIfPending();
	}
}

void APolyZone_Visualizer::RebuildMesh(UDynamicMesh* TargetMesh)
{
	if(IsValid(TargetMesh) && PolygonVertices.Num() >= 3)
	{
		FGeometryScriptPrimitiveOptions PrimitiveOptions;
		PrimitiveOptions.PolygroupMode = EGeometryScriptPrimitivePolygroupMode::PerFace;
		PrimitiveOptions.bFlipOrientation = true;
		PrimitiveOptions.UVMode = EGeometryScriptPrimitiveUVMode::Uniform;
		
		FTransform Transform;

		UGeometryScriptLibrary_MeshPrimitiveFunctions::AppendSimpleExtrudePolygon(TargetMesh, PrimitiveOptions, Transform, PolygonVertices, PolyZoneHeight, 0, false);
	}
}


