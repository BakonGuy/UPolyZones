// Copyright Seven47 Software All Rights Reserved.


#include "ZoneActor.h"


AZoneActor::AZoneActor()
{
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AZoneActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AZoneActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

