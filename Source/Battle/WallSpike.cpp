// Fill out your copyright notice in the Description page of Project Settings.


#include "WallSpike.h"
AWallSpike::AWallSpike() {
	PrimaryActorTick.bCanEverTick = true;
}

void AWallSpike::BeginPlay() {
	Super::BeginPlay();

	this->GetRootComponent()->ComponentVelocity = FVector(25, 0, 0);
}

void AWallSpike::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
	SetActorLocation(GetActorLocation() + FVector(350 * DeltaTime, 0, 0), true);
}

