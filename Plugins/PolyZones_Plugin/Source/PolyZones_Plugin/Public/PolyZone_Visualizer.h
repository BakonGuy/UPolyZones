// Copyright Seven47 Software All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GeometryActors/GeneratedDynamicMeshActor.h"
#include "PolyZone_Visualizer.generated.h"

UCLASS(Transient)
class APolyZone_Visualizer : public AGeneratedDynamicMeshActor
{
	GENERATED_BODY()
private:
	/** perform initial setup */
	virtual void PostInitProperties () override;

protected:
	virtual void ExecuteRebuildGeneratedMeshIfPending() override;

public:
	void RebuildMesh(UDynamicMesh* TargetMesh);

	TArray<FVector2D> PolygonVertices;
	float PolyZoneHeight = 0.0f;
	
};

template <typename ObjClass>
static FORCEINLINE ObjClass* LoadObjFromPath(const FName& Path)
{
	if (Path == NAME_None) return nullptr;

	return Cast<ObjClass>(StaticLoadObject(ObjClass::StaticClass(), nullptr, *Path.ToString()));
}

static FORCEINLINE UMaterial* LoadMaterialFromPath(const FName& Path)
{
	if (Path == NAME_None) return nullptr;

	return LoadObjFromPath<UMaterial>(Path);
}
