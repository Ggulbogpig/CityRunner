// Fill out your copyright notice in the Description page of Project Settings.


#include "Fountain.h"



// Sets default values
AFountain::AFountain()
{

 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BODY"));
	Water = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WATER"));
	Light = CreateDefaultSubobject<UPointLightComponent>(TEXT("Light"));
	Splash = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Splash"));
	Movement = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("MOVEMENT"));

	RootComponent = Body;
	Water->SetupAttachment(Body);
	Light->SetupAttachment(Body);
	Splash->SetupAttachment(Body);
	RotateSpeed = 50.0f;
	Movement->RotationRate = FRotator(0.0f, RotateSpeed, 0.0f);

	Water->SetRelativeLocation(FVector(0.0f, 0.0f, 135.0f));
	Body->SetRelativeScale3D(FVector(0.005f, 0.005f, 0.005f));
	Light->SetRelativeLocation(FVector(0.0f, 0.0f, 195.0f));
	Splash->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh>
		SM_BODY(TEXT("/Script/Engine.StaticMesh'/Game/fountain_water_simulation/h_1Shape1.h_1Shape1'"));

	if (SM_BODY.Succeeded()) {
		Body->SetStaticMesh(SM_BODY.Object);
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh>
		SM_WATER(TEXT("/Script/Engine.StaticMesh'/Game/fountain_water_simulation/fShape1.fShape1'"));

	if (SM_WATER.Succeeded()) {
		Water->SetStaticMesh(SM_WATER.Object);
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh>
		SM_SPLASH(TEXT("/Script/Engine.StaticMesh'/Game/fountain_water_simulation/f_2Shape1.f_2Shape1'"));

	if (SM_SPLASH.Succeeded()) {
		Splash->SetStaticMesh(SM_SPLASH.Object);
	}
	
}

// Called when the game starts or when spawned
void AFountain::BeginPlay()
{
	Super::BeginPlay();

	//UE_LOG(Battle, Warning, TEXT("ActorName: %s, ID: %d, Location X: %.3f"), *GetName(), ID, GetActorLocation().X);
	ABLOG_S(Warning);
	ABLOG(Warning, TEXT("Actor Name: %s, ID: %d, Location X: %.3f"), *GetName(), ID, GetActorLocation().X);
	
}

// Called every frame
void AFountain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	AddActorLocalRotation(FRotator(0.0f, RotateSpeed * DeltaTime, 0.0f));

}

void AFountain::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	Super::EndPlay(EndPlayReason);
	ABLOG_S(Warning);
}

void AFountain::PostInitializeComponents() {
	Super::PostInitializeComponents();
	ABLOG_S(Warning);
}

