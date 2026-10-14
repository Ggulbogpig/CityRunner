// Copyright Epic Games, Inc. All Rights Reserved.

#include "BattleGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "BattleCharacter.h"
#include "UObject/ConstructorHelpers.h"

ABattleGameMode::ABattleGameMode()
{
	// set default pawn class to our Blueprinted character
	//static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter1"));
	CurrentScore = 0;
	ScoreTickAmount = 5;
	ScoreInterval = 1.0f;
	//Game/Characters/BP_Character
	//Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter1
	//if (PlayerPawnBPClass.Class != NULL)
	//{
	//	DefaultPawnClass = PlayerPawnBPClass.Class;
	//}
	//생성자에선 경로만 지정
	PlayerPawnClass = TSoftClassPtr<APawn>(
		FSoftObjectPath(
			TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter1.BP_ThirdPersonCharacter1_C")
		)
	);
}

void ABattleGameMode::IncrementCoin()
{
	CurrentCoin += 1;
}

void ABattleGameMode::BeginPlay() {
	Super::BeginPlay();


	// 게임 시작 시 타이머 세팅
// 1.0초(ScoreInterval)마다 반복(true)해서 IncrementScoreOverTime 함수를 호출합니다.
	GetWorldTimerManager().SetTimer(
		ScoreTimerHandle,
		this,
		&ABattleGameMode::IncrementScoreOverTime,
		ScoreInterval,
		true
	);
}

void ABattleGameMode::IncrementScoreOverTime()
{
	// 점수 누적
	CurrentScore += ScoreTickAmount;

	// 로그 출력 (디버깅용)
	UE_LOG(LogTemp, Warning, TEXT("Running Score: %d"), CurrentScore);

	const int NewDifficultyLevel = CurrentScore / 50;

	// 맵 레벨 전환
	if (CurrentScore >= 30 && MapLevel == 1)
	{
		MapLevel = 2;

		UE_LOG(LogTemp, Warning,
			TEXT("MAP LEVEL UP -> LV2")
		);
	}

	// 아직 같은 50점 구간이면 아무것도 안 함
	if (NewDifficultyLevel == DifficultyLevel)
	{
		return;
	}

	DifficultyLevel = NewDifficultyLevel;

	ABattleCharacter* Player =
		Cast<ABattleCharacter>(
			UGameplayStatics::GetPlayerCharacter(this, 0)
		);

	if (Player)
	{
		Player->UpdateRunSpeed(DifficultyLevel);
		UE_LOG(LogTemp, Warning, TEXT("Levelup"))
	}
}

void ABattleGameMode::GameOver()
{
	// 플레이어가 죽거나 게임이 끝나면 타이머를 명확히 해제하여 점수 상승을 멈춥니다.
	GetWorldTimerManager().ClearTimer(ScoreTimerHandle);

	UE_LOG(LogTemp, Error, TEXT("Game Over! Final Score: %d"), CurrentScore);
}



int32 ABattleGameMode::GetCurrentScore() const
{
	return CurrentScore;
}

