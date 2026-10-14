// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BattleGameMode.generated.h"

UCLASS(minimalapi)
class ABattleGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ABattleGameMode();

	UPROPERTY(BlueprintReadOnly, Category="Score")
	int32 CurrentScore;

	void IncrementScoreOverTime();

	// 2. 게임 오버 시 타이머를 멈추기 위한 함수
	UFUNCTION(BlueprintCallable, Category = "Game")
	void GameOver();

	UFUNCTION(BlueprintCallable, Category = "Score")
	int32 GetCurrentScore() const;

	UPROPERTY(BlueprintReadOnly, Category = "Score")
	int32 CurrentCoin;

	UFUNCTION(BlueprintCallable, Category = "Score")
	void IncrementCoin();

	UPROPERTY(BlueprintReadOnly, Category = "Level")
	int32 MapLevel = 1;

	UPROPERTY()
	TSoftClassPtr<APawn> PlayerPawnClass;

protected:
	// 게임 시작 시 타이머를 가동하기 위해 BeginPlay 오버라이드
	virtual void BeginPlay() override;


private:
	// 3. 언리얼 엔진에서 타이머를 제어하기 위한 핸들 구조체
	FTimerHandle ScoreTimerHandle;

	// 초당 획득할 점수
	int32 ScoreTickAmount;

	// 점수가 증가하는 시간 간격 (예: 0.1초마다 혹은 1초마다)
	float ScoreInterval;

	int DifficultyLevel = 0;

};



