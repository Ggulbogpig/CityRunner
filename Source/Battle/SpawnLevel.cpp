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


	SpawnStartLevel(StartLevel);
	SpawnStartLevel(StartLevel1);
    SpawnLevel(false);
	SpawnLevel(false);
	
}

// Called every frame
void ASpawnLevel::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASpawnLevel::SpawnStartLevel(TSubclassOf<ABaseLevel> StartLevelClass)
{
    SpawnLocation = FVector(0.0f, 0.0f, 0.0f);
    SpawnRotation = FRotator(0, 0, 0);

    // 이전 청크가 있으면 그 청크의 SpawnLocation에서 이어서 생성
    if (LevelList.Num() > 0 && IsValid(LevelList.Last()))
    {
        ABaseLevel* LastLevel = LevelList.Last();

        if (LastLevel->GetSpawnLocation())
        {
            SpawnLocation =
                LastLevel->GetSpawnLocation()
                ->GetComponentTransform()
                .GetTranslation();
        }
    }

    if (!StartLevelClass)
    {
        UE_LOG(LogTemp, Error,
            TEXT("StartLevelClass is null"));
        return;
    }

    ABaseLevel* NewLevel =
        GetWorld()->SpawnActor<ABaseLevel>(
            StartLevelClass,
            SpawnLocation,
            SpawnRotation,
            SpawnInfo
        );

    if (!NewLevel)
    {
        UE_LOG(LogTemp, Error,
            TEXT("Failed to spawn StartLevel"));
        return;
    }

    OnLevelSpawned(NewLevel);

    if (NewLevel->GetTrigger())
    {
        NewLevel->GetTrigger()
            ->OnComponentBeginOverlap
            .AddDynamic(this, &ASpawnLevel::OnOverlapBegin);
    }

    LevelList.Add(NewLevel);
}

void ASpawnLevel::SpawnLevel(bool IsFirst)
{
    UE_LOG(LogTemp, Warning,
        TEXT("SpawnLevel called / count before: %d"),
        LevelList.Num());

    SpawnLocation = FVector(0.0f, 0.0f, 0.0f);
    SpawnRotation = FRotator(0.0f, 0.0f, 0.0f);

    ABattleGameMode* GameMode =
        Cast<ABattleGameMode>(
            UGameplayStatics::GetGameMode(this)
        );

    // -------------------------------------------------
    // 첫 청크가 아니라면 마지막 청크의 SpawnLocation 사용
    // -------------------------------------------------
    if (!IsFirst)
    {
        if (LevelList.Num() == 0 || !IsValid(LevelList.Last()))
        {
            UE_LOG(LogTemp, Error,
                TEXT("LevelList is empty or LastLevel is invalid"));
            return;
        }

        ABaseLevel* LastLevel = LevelList.Last();

        if (!LastLevel->GetSpawnLocation())
        {
            UE_LOG(LogTemp, Error,
                TEXT("LastLevel SpawnLocation is null"));
            return;
        }

        SpawnLocation =
            LastLevel->GetSpawnLocation()
            ->GetComponentTransform()
            .GetTranslation();
    }


    ABaseLevel* NewLevel = nullptr;


    // =================================================
    // 최초 시작 청크
    // =================================================
    if (IsFirst)
    {
        NewLevel = GetWorld()->SpawnActor<ABaseLevel>(
            StartLevel,
            SpawnLocation,
            SpawnRotation,
            SpawnInfo
        );
    }


    // =================================================
    // 일반 / 전환 청크
    // =================================================
    else
    {

        int32 CurrentMapLevel = 1;

        if (GameMode)
        {
            CurrentMapLevel = GameMode->MapLevel;
        }


        UE_LOG(LogTemp, Warning,
            TEXT("MapLevel Check / Current=%d / Last=%d"),
            CurrentMapLevel,
            LastMapLevel);


        // =================================================
        // LV1 -> LV2 전환
        // =================================================
        if (LastMapLevel == 1 && CurrentMapLevel == 2)
        {
            UE_LOG(LogTemp, Warning,
                TEXT("TRANSITION SPAWN 1 -> 2"));

            NewLevel = GetWorld()->SpawnActor<ABaseLevel>(
                TransitionLevel1To2,
                SpawnLocation,
                SpawnRotation,
                SpawnInfo
            );

            // 전환 청크 생성 성공 시에만 실제 생성 레벨 변경
            if (NewLevel)
            {
                LastMapLevel = 2;
            }
        }

        // =================================================
        // LV3 -> LV4 전환
        // =================================================
        else if (LastMapLevel == 3 && CurrentMapLevel == 4)
        {
            UE_LOG(LogTemp, Warning,
                TEXT("TRANSITION SPAWN 3 -> 4"));

            NewLevel = GetWorld()->SpawnActor<ABaseLevel>(
                TransitionLevel3To4,
                SpawnLocation,
                SpawnRotation,
                SpawnInfo
            );

            // 전환 청크 생성 성공 시에만 실제 생성 레벨 변경
            if (NewLevel)
            {
                LastMapLevel = 4;
            }
        }

        // =================================================
        // 그 외 레벨 변화
        // 현재는 별도 Transition이 없으므로 바로 전환
        // =================================================
        else
        {
            if (CurrentMapLevel != LastMapLevel)
            {
                UE_LOG(LogTemp, Warning,
                    TEXT("MAP LEVEL CHANGE %d -> %d"),
                    LastMapLevel,
                    CurrentMapLevel);

                LastMapLevel = CurrentMapLevel;
            }


            // =================================================
            // 일반 청크 생성
            // =================================================

            RandomLevel = FMath::RandRange(1, 9);


            // -------------------------------------------------
            // Level 1
            // -------------------------------------------------
            if (LastMapLevel == 1)
            {
                if (RandomLevel == 1)
                {
                    NewLevel = GetWorld()->SpawnActor<ABaseLevel>(
                        Level1,
                        SpawnLocation,
                        SpawnRotation,
                        SpawnInfo);
                }
                else if (RandomLevel == 2)
                {
                    NewLevel = GetWorld()->SpawnActor<ABaseLevel>(
                        Level2,
                        SpawnLocation,
                        SpawnRotation,
                        SpawnInfo);
                }
                else if (RandomLevel == 3)
                {
                    NewLevel = GetWorld()->SpawnActor<ABaseLevel>(
                        Level3,
                        SpawnLocation,
                        SpawnRotation,
                        SpawnInfo);
                }
                else if (RandomLevel == 4)
                {
                    NewLevel = GetWorld()->SpawnActor<ABaseLevel>(
                        Level4,
                        SpawnLocation,
                        SpawnRotation,
                        SpawnInfo);
                }
                else if (RandomLevel == 5)
                {
                    NewLevel = GetWorld()->SpawnActor<ABaseLevel>(
                        Level5,
                        SpawnLocation,
                        SpawnRotation,
                        SpawnInfo);
                }
                else if (RandomLevel == 6)
                {
                    NewLevel = GetWorld()->SpawnActor<ABaseLevel>(
                        Level6,
                        SpawnLocation,
                        SpawnRotation,
                        SpawnInfo);
                }
                else if (RandomLevel == 7)
                {
                    NewLevel = GetWorld()->SpawnActor<ABaseLevel>(
                        Level7,
                        SpawnLocation,
                        SpawnRotation,
                        SpawnInfo);
                }
                else if (RandomLevel == 8)
                {
                    NewLevel = GetWorld()->SpawnActor<ABaseLevel>(
                        Level8,
                        SpawnLocation,
                        SpawnRotation,
                        SpawnInfo);
                }
                else if (RandomLevel == 9)
                {
                    NewLevel = GetWorld()->SpawnActor<ABaseLevel>(
                        Level9,
                        SpawnLocation,
                        SpawnRotation,
                        SpawnInfo);
                }
            }


            // -------------------------------------------------
            // Level 2
            // -------------------------------------------------
            else if (LastMapLevel == 2)
            {
                if (RandomLevel == 1)
                {
                    NewLevel = GetWorld()->SpawnActor<ABaseLevel>(
                        Level11,
                        SpawnLocation,
                        SpawnRotation,
                        SpawnInfo);
                }
                else if (RandomLevel == 2)
                {
                    NewLevel = GetWorld()->SpawnActor<ABaseLevel>(
                        Level12,
                        SpawnLocation,
                        SpawnRotation,
                        SpawnInfo);
                }
                else if (RandomLevel == 3)
                {
                    NewLevel = GetWorld()->SpawnActor<ABaseLevel>(
                        Level13,
                        SpawnLocation,
                        SpawnRotation,
                        SpawnInfo);
                }
                else if (RandomLevel == 4)
                {
                    NewLevel = GetWorld()->SpawnActor<ABaseLevel>(
                        Level14,
                        SpawnLocation,
                        SpawnRotation,
                        SpawnInfo);
                }
                else if (RandomLevel == 5)
                {
                    NewLevel = GetWorld()->SpawnActor<ABaseLevel>(
                        Level15,
                        SpawnLocation,
                        SpawnRotation,
                        SpawnInfo);
                }
                else if (RandomLevel == 6)
                {
                    NewLevel = GetWorld()->SpawnActor<ABaseLevel>(
                        Level16,
                        SpawnLocation,
                        SpawnRotation,
                        SpawnInfo);
                }
                else if (RandomLevel == 7)
                {
                    NewLevel = GetWorld()->SpawnActor<ABaseLevel>(
                        Level17,
                        SpawnLocation,
                        SpawnRotation,
                        SpawnInfo);
                }
                else if (RandomLevel == 8)
                {
                    NewLevel = GetWorld()->SpawnActor<ABaseLevel>(
                        Level18,
                        SpawnLocation,
                        SpawnRotation,
                        SpawnInfo);
                }
                else if (RandomLevel == 9)
                {
                    NewLevel = GetWorld()->SpawnActor<ABaseLevel>(
                        Level19,
                        SpawnLocation,
                        SpawnRotation,
                        SpawnInfo);
                }
            }


            // -------------------------------------------------
            // Level 3
            // -------------------------------------------------
            else if (LastMapLevel == 3)
            {
                RandomLevel = FMath::RandRange(1, 5);

                if (RandomLevel == 1)
                {
                    NewLevel = GetWorld()->SpawnActor<ABaseLevel>(
                        Level20,
                        SpawnLocation,
                        SpawnRotation,
                        SpawnInfo);
                }
                else if (RandomLevel == 2)
                {
                    NewLevel = GetWorld()->SpawnActor<ABaseLevel>(
                        Level21,
                        SpawnLocation,
                        SpawnRotation,
                        SpawnInfo);
                }
                else if (RandomLevel == 3)
                {
                    NewLevel = GetWorld()->SpawnActor<ABaseLevel>(
                        Level22,
                        SpawnLocation,
                        SpawnRotation,
                        SpawnInfo);
                }
                else if (RandomLevel == 4)
                {
                    NewLevel = GetWorld()->SpawnActor<ABaseLevel>(
                        Level23,
                        SpawnLocation,
                        SpawnRotation,
                        SpawnInfo);
                }
                else if (RandomLevel == 5)
                {
                    NewLevel = GetWorld()->SpawnActor<ABaseLevel>(
                        Level24,
                        SpawnLocation,
                        SpawnRotation,
                        SpawnInfo);
                }
            }


            // -------------------------------------------------
            // Level 4
            // -------------------------------------------------
            else if (LastMapLevel == 4)
            {
                RandomLevel = FMath::RandRange(1, 5);

                if (RandomLevel == 1)
                {
                    NewLevel = GetWorld()->SpawnActor<ABaseLevel>(
                        Level30,
                        SpawnLocation,
                        SpawnRotation,
                        SpawnInfo);
                }
                else if (RandomLevel == 2)
                {
                    NewLevel = GetWorld()->SpawnActor<ABaseLevel>(
                        Level31,
                        SpawnLocation,
                        SpawnRotation,
                        SpawnInfo);
                }
                else if (RandomLevel == 3)
                {
                    NewLevel = GetWorld()->SpawnActor<ABaseLevel>(
                        Level32,
                        SpawnLocation,
                        SpawnRotation,
                        SpawnInfo);
                }
                else if (RandomLevel == 4)
                {
                    NewLevel = GetWorld()->SpawnActor<ABaseLevel>(
                        Level33,
                        SpawnLocation,
                        SpawnRotation,
                        SpawnInfo);
                }
                else if (RandomLevel == 5)
                {
                    NewLevel = GetWorld()->SpawnActor<ABaseLevel>(
                        Level34,
                        SpawnLocation,
                        SpawnRotation,
                        SpawnInfo);
                }
            }


            // -------------------------------------------------
            // Level 5
            // -------------------------------------------------
            else if (LastMapLevel == 5)
            {
                RandomLevel = FMath::RandRange(1, 5);

                if (RandomLevel == 1)
                {
                    NewLevel = GetWorld()->SpawnActor<ABaseLevel>(
                        Level40,
                        SpawnLocation,
                        SpawnRotation,
                        SpawnInfo);
                }
                else if (RandomLevel == 2)
                {
                    NewLevel = GetWorld()->SpawnActor<ABaseLevel>(
                        Level41,
                        SpawnLocation,
                        SpawnRotation,
                        SpawnInfo);
                }
                else if (RandomLevel == 3)
                {
                    NewLevel = GetWorld()->SpawnActor<ABaseLevel>(
                        Level42,
                        SpawnLocation,
                        SpawnRotation,
                        SpawnInfo);
                }
                else if (RandomLevel == 4)
                {
                    NewLevel = GetWorld()->SpawnActor<ABaseLevel>(
                        Level43,
                        SpawnLocation,
                        SpawnRotation,
                        SpawnInfo);
                }
                else if (RandomLevel == 5)
                {
                    NewLevel = GetWorld()->SpawnActor<ABaseLevel>(
                        Level44,
                        SpawnLocation,
                        SpawnRotation,
                        SpawnInfo);
                }
            }
        }
    }


    // =================================================
    // Spawn 실패 검사
    // =================================================
    if (!NewLevel)
    {
        UE_LOG(LogTemp, Error,
            TEXT("Level Spawn Failed! IsFirst=%d / LastMapLevel=%d"),
            IsFirst,
            LastMapLevel);

        return;
    }


    // =================================================
    // Spawn 이후 처리
    // =================================================
    OnLevelSpawned(NewLevel);


    if (NewLevel->GetTrigger())
    {
        NewLevel->GetTrigger()
            ->OnComponentBeginOverlap
            .AddDynamic(
                this,
                &ASpawnLevel::OnOverlapBegin
            );
    }


    LevelList.Add(NewLevel);


    if (GameMode && GameMode->DisplayMapLevel != GameMode->MapLevel)
    {
        GameMode->LevelEnterChunkCount++;

        if (GameMode->LevelEnterChunkCount >= 5)
        {
            GameMode->DisplayMapLevel = GameMode->PendingMapLevel;
            GameMode->LevelEnterChunkCount = 0;
        }
    }


    // =================================================
    // 오래된 청크 제거
    // =================================================
    if (LevelList.Num() > 5)
    {
        ABaseLevel* OldLevel = LevelList[0];

        if (IsValid(OldLevel))
        {
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




