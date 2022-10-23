// Copyright Seven47 Software All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ZoneActor.generated.h"

UCLASS(Abstract)
class POLYZONES_PLUGIN_API AZoneActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AZoneActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
