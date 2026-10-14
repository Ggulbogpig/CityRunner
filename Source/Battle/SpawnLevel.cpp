// Fill out your copyright notice in the Description page of Project Settings.


#include "SpawnLevel.h"
#include "BaseLevel.h"
#include "Engine.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include "BattleGameMode.h"
#include "BattleCharacter.h"

// Sets default values
ASpawnLevel::ASpawnLevel()
{

 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;



}

// Called when the game starts or when spawned
void ASpawnLevel::BeginPlay()
{
	Super::BeginPlay();


	SpawnLevel(true);
	SpawnLevel(false);
	SpawnLevel(false);
	SpawnLevel(false);
	
}

// Called every frame
void ASpawnLevel::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASpawnLevel::SpawnLevel(bool IsFirst)
{
	UE_LOG(LogTemp, Warning,
		TEXT("SpawnLevel called / count before: %d"),
		LevelList.Num());

	SpawnLocation = FVector(0.0f, 0.0f, 0.0f);
	SpawnRotation = FRotator(0, 0, 0);

	if (!IsFirst) {
		ABaseLevel* LastLevel = LevelList.Last();
		SpawnLocation = LastLevel->GetSpawnLocation()->GetComponentTransform().GetTranslation();

		//UE_LOG(LogTemp, Warning,
		//	TEXT("NextSpawn = %s"),
		//	*SpawnLocation.ToString());
	}
	
	//레벨 난이도 선정
	ABattleGameMode* GameMode =
		Cast<ABattleGameMode>(
			UGameplayStatics::GetGameMode(this)
		);

	int32 CurrentMapLevel = 1;

	if (GameMode)
	{
		CurrentMapLevel = GameMode->MapLevel;
	}


	RandomLevel = FMath::RandRange(1, 9);
	ABaseLevel* NewLevel = nullptr;

	//난이도 레벨1 
	if (CurrentMapLevel == 1) {
		if (RandomLevel == 1) {
			NewLevel = GetWorld()->SpawnActor<ABaseLevel>(Level1,
				SpawnLocation, SpawnRotation, SpawnInfo);
		}
		else if (RandomLevel == 2) {
			NewLevel = GetWorld()->SpawnActor<ABaseLevel>(Level2,
				SpawnLocation, SpawnRotation, SpawnInfo);
		}
		else if (RandomLevel == 3) {
			NewLevel = GetWorld()->SpawnActor<ABaseLevel>(Level3,
				SpawnLocation, SpawnRotation, SpawnInfo);
		}
		else if (RandomLevel == 4) {
			NewLevel = GetWorld()->SpawnActor<ABaseLevel>(Level4,
				SpawnLocation, SpawnRotation, SpawnInfo);
		}
		else if (RandomLevel == 5) {
			NewLevel = GetWorld()->SpawnActor<ABaseLevel>(Level5,
				SpawnLocation, SpawnRotation, SpawnInfo);
		}
		else if (RandomLevel == 6) {
			NewLevel = GetWorld()->SpawnActor<ABaseLevel>(Level6,
				SpawnLocation, SpawnRotation, SpawnInfo);
		}
		else if (RandomLevel == 7) {
			NewLevel = GetWorld()->SpawnActor<ABaseLevel>(Level7,
				SpawnLocation, SpawnRotation, SpawnInfo);
		}
		else if (RandomLevel == 8) {
			NewLevel = GetWorld()->SpawnActor<ABaseLevel>(Level8,
				SpawnLocation, SpawnRotation, SpawnInfo);
		}
		else if (RandomLevel == 9) {
			NewLevel = GetWorld()->SpawnActor<ABaseLevel>(Level9,
				SpawnLocation, SpawnRotation, SpawnInfo);
		}

	}

	//난이도 레벨 2
	else if (CurrentMapLevel == 2) {
		if (RandomLevel == 1) {
			NewLevel = GetWorld()->SpawnActor<ABaseLevel>(Level11,
				SpawnLocation, SpawnRotation, SpawnInfo);
		}
		else if (RandomLevel == 2) {
			NewLevel = GetWorld()->SpawnActor<ABaseLevel>(Level12,
				SpawnLocation, SpawnRotation, SpawnInfo);
		}
		else if (RandomLevel == 3) {
			NewLevel = GetWorld()->SpawnActor<ABaseLevel>(Level13,
				SpawnLocation, SpawnRotation, SpawnInfo);
		}
		else if (RandomLevel == 4) {
			NewLevel = GetWorld()->SpawnActor<ABaseLevel>(Level14,
				SpawnLocation, SpawnRotation, SpawnInfo);
		}
		else if (RandomLevel == 5) {
			NewLevel = GetWorld()->SpawnActor<ABaseLevel>(Level15,
				SpawnLocation, SpawnRotation, SpawnInfo);
		}
		else if (RandomLevel == 6) {
			NewLevel = GetWorld()->SpawnActor<ABaseLevel>(Level16,
				SpawnLocation, SpawnRotation, SpawnInfo);
		}
		else if (RandomLevel == 7) {
			NewLevel = GetWorld()->SpawnActor<ABaseLevel>(Level17,
				SpawnLocation, SpawnRotation, SpawnInfo);
		}
		else if (RandomLevel == 8) {
			NewLevel = GetWorld()->SpawnActor<ABaseLevel>(Level18,
				SpawnLocation, SpawnRotation, SpawnInfo);
		}
		else if (RandomLevel == 9) {
			NewLevel = GetWorld()->SpawnActor<ABaseLevel>(Level19,
				SpawnLocation, SpawnRotation, SpawnInfo);
		}
	}


	if (NewLevel) {

		//UE_LOG(LogTemp, Warning,
		//	TEXT("Spawned Location = %s"),
		//	*NewLevel->GetActorLocation().ToString());

		OnLevelSpawned(NewLevel);

		if (NewLevel->GetTrigger()) {
			NewLevel->GetTrigger()->OnComponentBeginOverlap.AddDynamic(this, &ASpawnLevel::OnOverlapBegin);
		}
	}

	LevelList.Add(NewLevel);
	if (LevelList.Num() > 5) {
		ABaseLevel* OldLevel = LevelList[0];

		if (IsValid(OldLevel))
		{
			//UE_LOG(LogTemp, Warning,
			//	TEXT("DESTROY LEVEL: %s"),
			//	*GetNameSafe(LevelList[0]));
			OldLevel->Destroy();
		}
		LevelList.RemoveAt(0);
	}
}

//void ASpawnLevel::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
//{
//
//	
//	if (Cast<ABattleCharacter>(OtherActor))
//	{
//		UE_LOG(LogTemp, Warning,
//			TEXT("TRIGGER OWNER=%s / OtherComp=%s / LevelCount=%d"),
//			*GetNameSafe(OverlappedComp ? OverlappedComp->GetOwner() : nullptr),
//			*GetNameSafe(OtherComp),
//			LevelList.Num());
//
//
//		SpawnLevel(false);
//	}
//
//}

void ASpawnLevel::OnOverlapBegin(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (Cast<ABattleCharacter>(OtherActor))
	{
		//UE_LOG(LogTemp, Warning,
		//	TEXT("TRIGGER OWNER=%s / OtherComp=%s / LevelCount=%d"),
		//	*GetNameSafe(OverlappedComp ? OverlappedComp->GetOwner() : nullptr),
		//	*GetNameSafe(OtherComp),
		//	LevelList.Num());

		// 이 Trigger는 한 번만 사용
		if (OverlappedComp)
		{
			OverlappedComp->SetGenerateOverlapEvents(false);
			OverlappedComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}

		SpawnLevel(false);
	}
}




