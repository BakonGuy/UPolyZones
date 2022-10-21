// Copyright Seven47 Software All Rights Reserved.

#include "PolyZone_Visualizer.h"
#include "GeometryScript/MeshPrimitiveFunctions.h"

void APolyZone_Visualizer::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if(IsValid(DynamicMeshComponent))
	{
		static ConstructorHelpers::FObjectFinder<UMaterialInterface> DefaultMaterial(TEXT("/PolyZones_Plugin/PolyZoneDebugMaterial.PolyZoneDebugMaterial'"));
		if (DefaultMaterial.Succeeded())
		{
			UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(DefaultMaterial.Object, this,FName("M_CreatedInstance"));
			DynamicMeshComponent->SetMaterial(0, DynamicMaterial);
		}
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


