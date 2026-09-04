// Copyright Epic Games, Inc. All Rights Reserved.

#include "BattleCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

#include "BattleGameMode.h"
#include "Spikes.h"
#include "WallSpike.h"
#include "Engine.h"
DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ABattleCharacter

ABattleCharacter::ABattleCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 900.f;
	GetCharacterMovement()->AirControl = 2.5f; //공중에서 얼마나 이동을 자유롭게 하는지
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	JumpMaxCount = 2;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void ABattleCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	RunDirection = GetActorForwardVector().GetSafeNormal();
	BaseRunDirection = GetActorForwardVector();
	BaseRunDirection.Z = 0.0f;
	BaseRunDirection.Normalize();
	LateralInput = 0.0f;

	CurrentRunDirection = BaseRunDirection;

	CanMove = true;

	GetCharacterMovement()->MaxWalkSpeed = StartRunSpeed;

	

	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this,&ABattleCharacter::OnOverlapBegin);

	
}

//////////////////////////////////////////////////////////////////////////
// Input

void ABattleCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		
		EnhancedInputComponent->BindAction(
			MoveAction,
			ETriggerEvent::Triggered,
			this,
			&ABattleCharacter::Move
		);

		EnhancedInputComponent->BindAction(
			MoveAction,
			ETriggerEvent::Completed,
			this,
			&ABattleCharacter::StopMove
		);

		EnhancedInputComponent->BindAction(
			MoveAction,
			ETriggerEvent::Canceled,
			this,
			&ABattleCharacter::StopMove
		);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABattleCharacter::Look);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

//void ABattleCharacter::Move(const FInputActionValue& Value)
//{
//	// input is a Vector2D
//	FVector2D MovementVector = Value.Get<FVector2D>();
//
//	if (Controller != nullptr)
//	{
//		//추가
//	/*	if (bOnWall) {
//			return;
//		}*/
//		// find out which way is forward
//		const FRotator Rotation = Controller->GetControlRotation();
//		const FRotator YawRotation(0, Rotation.Yaw, 0);
//
//		// get forward vector
//		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
//	
//		// get right vector 
//		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
//
//		// add movement 
//		AddMovementInput(ForwardDirection, MovementVector.Y);
//		AddMovementInput(RightDirection, MovementVector.X);
//	}
//}

void ABattleCharacter::UpdateRunSpeed(int DifficultyLevel)
{
	const float NewSpeed =
		StartRunSpeed + (DifficultyLevel * SpeedIncreaseAmount);

	GetCharacterMovement()->MaxWalkSpeed =
		FMath::Min(NewSpeed, MaxRunSpeed);
}

//void ABattleCharacter::TriggerChaser()
//{
//	if (!ChaserEnemy)
//	{
//		return;
//	}
//
//	// 첫 번째 충돌
//	if (!bChaserActive)
//	{
//		bChaserActive = true;
//
//		CurrentChaserDistance = ChaserStartDistance;
//
//		ChaserEnemy->SetActorHiddenInGame(false);
//		ChaserEnemy->SetActorEnableCollision(true);
//
//		GetWorldTimerManager().ClearTimer(ChaserTimerHandle);
//
//		GetWorldTimerManager().SetTimer(
//			ChaserTimerHandle,
//			this,
//			&ABattleCharacter::HideChaser,
//			ChaserDuration,
//			false
//		);
//
//		return;
//	}
//
//	// 이미 추격 중인데 또 장애물에 부딪힘
//	CatchPlayer();
//}
//
//void ABattleCharacter::CatchPlayer()
//{
//	if (!ChaserEnemy)
//	{
//		return;
//	}
//
//	GetWorldTimerManager().ClearTimer(ChaserTimerHandle);
//
//	// 일단 플레이어 바로 뒤로 이동
//	const FVector CatchLocation =
//		GetActorLocation()
//		- BaseRunDirection * 100.0f;
//
//	ChaserEnemy->SetActorLocation(CatchLocation);
//
//	// 플레이어 정지
//	CanMove = false;
//
//	// 여기서 Game Over 처리
//}
//
//void ABattleCharacter::HideChaser()
//{
//	if (!ChaserEnemy) {
//		return;
//	}
//	bChaserActive = false;
//
//	ChaserEnemy->SetActorHiddenInGame(true);
//	ChaserEnemy->SetActorEnableCollision(false);
//
//	CurrentChaserDistance = ChaserStartDistance;
//}
//
//void ABattleCharacter::UpdateChaserPosition()
//{
//	if (!bChaserActive || !ChaserEnemy) {
//		return;
//	}
//
//	const FVector ChaserLocation =
//		GetActorLocation()
//		- BaseRunDirection * CurrentChaserDistance;
//}

void ABattleCharacter::DieAndRestart()
{
	//GetMesh()->Deactivate();
	//GetMesh()->SetVisibility(false);
	SetActorHiddenInGame(true);
	
	//CanMove = false;
	FTimerHandle UnusedHandle;
	GetWorldTimerManager().SetTimer(UnusedHandle, this,
		&ABattleCharacter::RestartLevel, 2.f, false);

}

void ABattleCharacter::Move(const FInputActionValue& Value)
{
	//테스트 모드
	//FVector2D MovementVector = Value.Get<FVector2D>();

	//if (Controller != nullptr)
	//{
	//	const FRotator Rotation = Controller->GetControlRotation();
	//	const FRotator YawRotation(0, Rotation.Yaw, 0);

	//	const FVector ForwardDirection =
	//		FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

	//	const FVector RightDirection =
	//		FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	//	AddMovementInput(ForwardDirection, MovementVector.Y);
	//	AddMovementInput(RightDirection, MovementVector.X);
	//}
	//자동전진모드
	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (!Controller)
	{
		LateralInput = 0.0f;
		return;
	}


	// 맵 회전 중에는 A/D 입력 무시
	if (bMapRotating)
	{
		LateralInput = 0.0f;
		return;
	}

	LateralInput = FMath::Clamp(
		MovementVector.X,
		-1.0f,
		1.0f
	);
}

void ABattleCharacter::StopMove(const FInputActionValue& Value)
{
	LateralInput = 0.0f;
}


//void ABattleCharacter::UpdateRunnerMovement(float DeltaTime)
//{
//
//
//	const FVector RightDirection =
//		FVector::CrossProduct(
//			FVector::UpVector,
//			BaseRunDirection
//		).GetSafeNormal();
//
//	// 자동 전진은 계속
//	AddMovementInput(BaseRunDirection, 1.0f);
//
//	if (GetCharacterMovement()->IsFalling())
//	{
//		FVector Velocity = GetCharacterMovement()->Velocity;
//
//		const float AirSideSpeed = 400.0f;
//
//		const float CurrentSideSpeed =
//			FVector::DotProduct(Velocity, RightDirection);
//
//		Velocity -= RightDirection * CurrentSideSpeed;
//		Velocity += RightDirection * (LateralInput * AirSideSpeed);
//
//		GetCharacterMovement()->Velocity = Velocity;
//	}
//	else
//	{
//		AddMovementInput(
//			RightDirection,
//			LateralInput * 0.75f
//		);
//	}
//
//	// 벽타기 중에는 플러그인이 Rotation을 담당하게 둠
//	if (!bOnWall)
//	{
//		const FRotator TargetRotation =
//			BaseRunDirection.Rotation();
//
//		const FRotator NewRotation =
//			FMath::RInterpTo(
//				GetActorRotation(),
//				TargetRotation,
//				DeltaTime,
//				10.0f
//			);
//
//		SetActorRotation(NewRotation);
//	}
//
//	const FVector Velocity = GetVelocity();
//	const float ForwardSpeed =
//		FVector::DotProduct(Velocity, BaseRunDirection);
//
//	UE_LOG(
//		LogTemp,
//		Warning,
//		TEXT("OnWall=%d TotalSpeed=%.1f ForwardSpeed=%.1f Velocity=%s"),
//		bOnWall,
//		Velocity.Size(),
//		ForwardSpeed,
//		*Velocity.ToString()
//	);
//
//	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
//
//	const FVector CurrentVelocity = MoveComp->Velocity;
//
//
//
//	const FVector Accel = MoveComp->GetCurrentAcceleration();
//	const FVector LastInput = GetLastMovementInputVector();
//
//	UE_LOG(
//		LogTemp,
//		Warning,
//		TEXT("[%s] Wall=%d Mode=%d Vel=%.1f Forward=%.1f "
//			"Accel=%s Input=%s MaxSpeed=%.1f Friction=%.2f Brake=%.1f"),
//		*GetName(),
//		bOnWall,
//		(int32)MoveComp->MovementMode,
//		CurrentVelocity.Size(),
//		ForwardSpeed,
//		*Accel.ToString(),
//		*LastInput.ToString(),
//		MoveComp->MaxWalkSpeed,
//		MoveComp->GroundFriction,
//		MoveComp->BrakingDecelerationWalking
//	);
//}

void ABattleCharacter::UpdateRunnerMovement(float DeltaTime)
{
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();

	// 자동 전진
	AddMovementInput(BaseRunDirection, 1.0f);

	if (bOnWall)
	{
		// 벽의 평면 안에서의 좌우 방향
		const FVector WallSideDirection =
			FVector::CrossProduct(
				CurrentWallNormal,
				BaseRunDirection
			).GetSafeNormal();

		AddMovementInput(
			WallSideDirection,
			LateralInput * 0.75f
		);
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("WALL MOVE | Lateral=%.2f Normal=%s Base=%s Side=%s"),
			LateralInput,
			*CurrentWallNormal.ToString(),
			*BaseRunDirection.ToString(),
			*WallSideDirection.ToString()
		);
	}
	else
	{
		const FVector RightDirection =
			FVector::CrossProduct(
				FVector::UpVector,
				BaseRunDirection
			).GetSafeNormal();

		if (MoveComp->IsFalling())
		{
			FVector Velocity = MoveComp->Velocity;

			const float AirSideSpeed = 400.0f;

			const float CurrentSideSpeed =
				FVector::DotProduct(Velocity, RightDirection);

			Velocity -= RightDirection * CurrentSideSpeed;
			Velocity += RightDirection *
				(LateralInput * AirSideSpeed);

			MoveComp->Velocity = Velocity;
		}
		else
		{
			AddMovementInput(
				RightDirection,
				LateralInput * 0.75f
			);
		}
	}

	// 이건 지금처럼 벽에서는 하지 않음
	if (!bOnWall)
	{
		const FRotator TargetRotation =
			BaseRunDirection.Rotation();

		const FRotator NewRotation =
			FMath::RInterpTo(
				GetActorRotation(),
				TargetRotation,
				DeltaTime,
				10.0f
			);

		SetActorRotation(NewRotation);
	}
}
void ABattleCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}
void ABattleCharacter::RestartLevel() {
	UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()));
}

void ABattleCharacter::OnOverlapBegin(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {

	if (OtherActor != nullptr) {
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Overlap 발생! OtherActor: %s, OtherComp: %s"),
			*GetNameSafe(OtherActor),
			*GetNameSafe(OtherComp)
		);
		ASpikes* WallSpike = Cast<AWallSpike>(OtherActor);
		ASpikes* Spike = Cast<ASpikes>(OtherActor);
		// BP_Level의 낙사 판정 Box인지 확인
		const FName ComponentName = OtherComp->GetFName();

		const bool bKillBox =
			ComponentName == TEXT("Die1") ||
			ComponentName == TEXT("Die2") ||
			ComponentName == TEXT("Die3") ||
			ComponentName == TEXT("Die4");
		

		if (bKillBox) {
			DieAndRestart();
		}
		else if (WallSpike || Spike) {
			OnTriggerChase();
			
		}


	}

}

void ABattleCharacter::StartWallTransition()
{
	LateralInput = 0.0f;
}

void ABattleCharacter::SetMapRotating(bool bRotating)
{
	bMapRotating = bRotating;

	if (bMapRotating)
	{
		LateralInput = 0.0f;
	}
}

void ABattleCharacter::Tick(float DeltaTime) {

	Super::Tick(DeltaTime);

	if (!CanMove)
	{
		return;
	}


	//AddMovementInput(MoveDirection, 1.0f);
	UpdateRunnerMovement(DeltaTime);

	//if (bChaserActive) {
	//	UpdateChaserPosition();
	//}

	


	

}