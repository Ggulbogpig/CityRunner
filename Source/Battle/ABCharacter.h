// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Battle.h"
//#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ABCharacter.generated.h"

UCLASS()
class BATTLE_API AABCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AABCharacter();
	//Ãß°¡
	UPROPERTY(BlueprintReadWrite, Category = "WallRun")
	bool bOnWall = false;



protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void HandleAutoRun();

	enum class EControlMode {
		GTA,
		DIABLO
	};

	enum class ERunnerState {
		GroundRunning,
		Jumping,
		Falling,
		Sliding,
		WallRunning,
		Grinding,
		Dead
	};



	void SetControlMode(EControlMode ControlMode);
	void SetRunnerState(ERunnerState RunnerState);
	EControlMode CurrentControlMode = EControlMode::DIABLO;
	ERunnerState CurrentRunnerState = ERunnerState::GroundRunning;
	FVector DirectionToMove = FVector::ZeroVector;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void PostInitializeComponents() override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere, Category=Camera)
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	UCameraComponent* Camera;

	UPROPERTY()
	TSoftClassPtr<UAnimInstance> WarriorAnimClass;

private:
	void UpDown(float NewAxisValue);
	void LeftRight(float NewAxisValue);
	void LookUp(float NewAxisValue);
	void Turn(float NewAxisValue);

	

	void ViewChange();
	void Attack();

	UFUNCTION()
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	void AttackStartComboState();
	void AttackEndComboState();

private:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = Attack, Meta = (AllowPrivateAccess = true))
	bool IsAttacking;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = Attack, Meta = (AllowPrivateAccess = true))
	bool CanNextCombo;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = Attack, Meta = (AllowPrivateAccess = true))
	bool IsComboInputOn;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = Attack, Meta = (AllowPrivateAccess = true))
	int32 CurrentCombo;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = Attack, Meta = (AllowPrivateAccess = true))
	int32 MaxCombo;


	UPROPERTY()
	class UABAnimInstance* ABAnim;
};
