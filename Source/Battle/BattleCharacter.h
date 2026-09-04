// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "BattleCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class ABattleCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;








private:
	FVector RunDirection;
	FVector BaseRunDirection;
	FVector CurrentRunDirection;
	float LateralInput;



public:
	ABattleCharacter();

	//추가
	UPROPERTY(BlueprintReadWrite, Category = "WallRun")
	bool bOnWall = false;
	bool CanMove = true;

	// BattleCharacter.h

	UPROPERTY(BlueprintReadWrite, Category = "Runner")
	bool bMapRotating = false;

	UFUNCTION(BlueprintCallable, Category = "Runner")
	void SetMapRotating(bool bRotating);

	
	// 속도 관련
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed")
	float StartRunSpeed = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed")
	float SpeedIncreaseAmount = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed")
	float MaxRunSpeed = 1200.0f;

	UFUNCTION(BlueprintCallable, Category = "Speed")
	void UpdateRunSpeed(int DifficultyLevel);

	UPROPERTY(BlueprintReadWrite, Category = "WallRun")
	FVector CurrentWallNormal = FVector::ZeroVector;

	//////////////////////////////////////////////////////////
	//추격자 클래스
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Runner|Chaser")
	//TSubclassOf<AActor> ChaserEnemyClass;
	//추격자
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chaser")
	//AActor* ChaserEnemy = nullptr;

	//처음 등장할 때 플레이어와의 거리
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chaser")
	//float ChaserStartDistance = 1000.0f;

	//추가 충돌 시 얼마나 가까워질지
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chaser")
	//float ChaserApproachAmount = 300.0f;

	//최소 거리
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chaser")
	//float ChaserMinDistance = 250.0f;

	//추격 유지 시간
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chaser")
	//float ChaserDuration = 5.0f;


	//UPROPERTY(BlueprintReadOnly, Category = "Chaser")
	//bool bChaserActive = false;

	//float CurrentChaserDistance = 1000.0f;

	//FTimerHandle ChaserTimerHandle;

	//UFUNCTION(BlueprintCallable, Category="Chaser")
	//void TriggerChaser();

	//void HideChaser();

	//void CatchPlayer();

	//void UpdateChaserPosition();

	UFUNCTION(BlueprintImplementableEvent, Category = "Chaser")
	void OnTriggerChase();

	UFUNCTION(BlueprintCallable, Category = "Runner")
	void DieAndRestart();
	


	

	
	

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	void StopMove(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	void UpdateRunnerMovement(
		float DeltaTime);

	UFUNCTION(BlueprintCallable, Category="Runner")
	void StartWallTransition();
	
			

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// To add mapping context
	virtual void BeginPlay();
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	void RestartLevel();

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

