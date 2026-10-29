// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpawnLevel.generated.h"

class ABaseLevel;

UCLASS()
class BATTLE_API ASpawnLevel : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASpawnLevel();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
public:

	UFUNCTION()
	void SpawnLevel(bool IsFirst);

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(BlueprintImplementableEvent)
	void OnLevelSpawned(ABaseLevel* NewLevel);

	UFUNCTION()
	void SpawnStartLevel(TSubclassOf<ABaseLevel> StartLevelClass);



protected:
	APawn* Player;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level")
	TSubclassOf<ABaseLevel> StartLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level")
	TSubclassOf<ABaseLevel> StartLevel1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level")
	TSubclassOf<ABaseLevel> StartLevel2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level")
	TSubclassOf<ABaseLevel> TransitionLevel1To2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level")
	TSubclassOf<ABaseLevel> TransitionLevel3To4;

	int32 LastMapLevel = 1;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ABaseLevel> Level1;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ABaseLevel> Level2;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ABaseLevel> Level3;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ABaseLevel> Level4;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ABaseLevel> Level5;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ABaseLevel> Level6;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ABaseLevel> Level7;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ABaseLevel> Level8;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ABaseLevel> Level9;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ABaseLevel> Level10;


	UPROPERTY(EditAnywhere)
	TSubclassOf<ABaseLevel> Level11;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ABaseLevel> Level12;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ABaseLevel> Level13;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ABaseLevel> Level14;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ABaseLevel> Level15;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ABaseLevel> Level16;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ABaseLevel> Level17;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ABaseLevel> Level18;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ABaseLevel> Level19;


	UPROPERTY(EditAnywhere)
	TSubclassOf<ABaseLevel> Level20;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ABaseLevel> Level21;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ABaseLevel> Level22;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ABaseLevel> Level23;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ABaseLevel> Level24;
	



	UPROPERTY(EditAnywhere)
	TSubclassOf<ABaseLevel> Level30;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ABaseLevel> Level31;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ABaseLevel> Level32;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ABaseLevel> Level33;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ABaseLevel> Level34;



	UPROPERTY(EditAnywhere)
	TSubclassOf<ABaseLevel> Level40;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ABaseLevel> Level41;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ABaseLevel> Level42;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ABaseLevel> Level43;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ABaseLevel> Level44;
	TArray<ABaseLevel*> LevelList;


public:
	int RandomLevel;
	bool bPendingTransition = false;

	FVector SpawnLocation = FVector();
	FRotator SpawnRotation = FRotator();
	FActorSpawnParameters SpawnInfo = FActorSpawnParameters();
};
