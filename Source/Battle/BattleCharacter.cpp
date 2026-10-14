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

	JumpMaxCount = 1;

	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;

	//카메라 회전 수정 전
	// Create a camera boom (pulls in towards the player if there is a collision)
	//CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	//CameraBoom->SetupAttachment(RootComponent);
	//CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	//CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	//// Create a follow camera
	//FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	//FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	//FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
	
	//카메라 회전 기준 수정 후
	// Camera Boom
	CameraBoom =
		CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));

	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;
	// Roll 전용 Pivot
	CameraRollRoot =
		CreateDefaultSubobject<USceneComponent>(TEXT("CameraRollRoot"));

	CameraRollRoot->SetupAttachment(
		CameraBoom,
		USpringArmComponent::SocketName
	);
	// Follow Camera
	FollowCamera =
		CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));

	FollowCamera->SetupAttachment(CameraRollRoot);
	FollowCamera->bUsePawnControlRotation = false;
	
	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
	//슬라이드 추가
	static ConstructorHelpers::FObjectFinder<UInputAction> SlideActionAsset(
		TEXT("/Game/IA_Slide.IA_Slide")
	);

	if (SlideActionAsset.Succeeded())
	{
		SlideAction = SlideActionAsset.Object;
	}

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
	//초기 캡슐 높이값 저장
	DefaultCapsuleHalfHeight = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();

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

		//EnhancedInputComponent->BindAction(
		//	SlideAction,
		//	ETriggerEvent::Started,
		//	this,
		//	&ABattleCharacter::StartSlide
		//);

		//EnhancedInputComponent->BindAction(
		//	SlideAction,
		//	ETriggerEvent::Completed,
		//	this,
		//	&ABattleCharacter::EndSlide
		//);

		//EnhancedInputComponent->BindAction(
		//	SlideAction,
		//	ETriggerEvent::Canceled,
		//	this,
		//	&ABattleCharacter::EndSlide
		//);
		if (SlideAction)
		{
			UE_LOG(LogTemp, Warning, TEXT("SlideAction VALID"));

			EnhancedInputComponent->BindAction(
				SlideAction,
				ETriggerEvent::Started,
				this,
				&ABattleCharacter::StartSlide
			);

			EnhancedInputComponent->BindAction(
				SlideAction,
				ETriggerEvent::Completed,
				this,
				&ABattleCharacter::EndSlide
			);

			EnhancedInputComponent->BindAction(
				SlideAction,
				ETriggerEvent::Canceled,
				this,
				&ABattleCharacter::EndSlide
			);

			EnhancedInputComponent->BindAction(
				TestGravityAction,
				ETriggerEvent::Started,
				this,
				&ABattleCharacter::RotateCameraRight
				//&ABattleCharacter::TestNextOrientation
			);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("SlideAction NULL"));
		}


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

void ABattleCharacter::SetTunnelOrientation(ETunnelOrientation NewOrientation)
{
	CurrentTunnelOrientation = NewOrientation;

	FVector NewGravityDirection;

	switch (CurrentTunnelOrientation)
	{
	case ETunnelOrientation::Floor:
		NewGravityDirection = FVector(0.f, 0.f, -1.f);
		break;

	case ETunnelOrientation::RightWall:
		NewGravityDirection = FVector(0.f, 1.f, 0.f);
		break;

	case ETunnelOrientation::Ceiling:
		NewGravityDirection = FVector(0.f, 0.f, 1.f);
		break;

	case ETunnelOrientation::LeftWall:
		NewGravityDirection = FVector(0.f, -1.f, 0.f);
		break;

	default:
		NewGravityDirection = FVector(0.f, 0.f, -1.f);
		break;
	}

	GetCharacterMovement()->SetGravityDirection(NewGravityDirection);



	//UE_LOG(LogTemp, Warning,
	//	TEXT("SetTunnelOrientation / Falling=%d / Gravity=%s / Velocity=%s"),
	//	GetCharacterMovement()->IsFalling(),
	//	*GetCharacterMovement()->GetGravityDirection().ToString(),
	//	*GetCharacterMovement()->Velocity.ToString());
}

void ABattleCharacter::RotateTunnelOrientationRight()
{
	switch (CurrentTunnelOrientation)
	{
	case ETunnelOrientation::Floor:
		SetTunnelOrientation(ETunnelOrientation::RightWall);
		break;

	case ETunnelOrientation::RightWall:
		SetTunnelOrientation(ETunnelOrientation::Ceiling);
		break;

	case ETunnelOrientation::Ceiling:
		SetTunnelOrientation(ETunnelOrientation::LeftWall);
		break;

	case ETunnelOrientation::LeftWall:
		SetTunnelOrientation(ETunnelOrientation::Floor);
		break;
	}
}

void ABattleCharacter::RotateTunnelOrientationLeft()
{
	switch (CurrentTunnelOrientation)
	{
	case ETunnelOrientation::Floor:
		SetTunnelOrientation(ETunnelOrientation::LeftWall);
		break;

	case ETunnelOrientation::LeftWall:
		SetTunnelOrientation(ETunnelOrientation::Ceiling);
		break;

	case ETunnelOrientation::Ceiling:
		SetTunnelOrientation(ETunnelOrientation::RightWall);
		break;

	case ETunnelOrientation::RightWall:
		SetTunnelOrientation(ETunnelOrientation::Floor);
		break;
	}
}

void ABattleCharacter::UnlockSurfaceChange()
{
	bSurfaceChanging = false;
}

void ABattleCharacter::TestRightGravity()
{
	SetTunnelOrientation(ETunnelOrientation::RightWall);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("TestRightGravity called")
	);
}

void ABattleCharacter::TestRightOrientation()
{
	switch (CurrentTunnelOrientation)
	{
	case ETunnelOrientation::Floor:
		SetTunnelOrientation(ETunnelOrientation::RightWall);
		break;

	case ETunnelOrientation::RightWall:
		SetTunnelOrientation(ETunnelOrientation::Ceiling);
		break;

	case ETunnelOrientation::Ceiling:
		SetTunnelOrientation(ETunnelOrientation::LeftWall);
		break;

	case ETunnelOrientation::LeftWall:
		SetTunnelOrientation(ETunnelOrientation::Floor);
		break;
	}
}

void ABattleCharacter::TestLeftOrientation()
{
	switch (CurrentTunnelOrientation)
	{
	case ETunnelOrientation::Floor:
		SetTunnelOrientation(ETunnelOrientation::RightWall);
		break;

	case ETunnelOrientation::LeftWall:
		SetTunnelOrientation(ETunnelOrientation::Ceiling);
		break;

	case ETunnelOrientation::Ceiling:
		SetTunnelOrientation(ETunnelOrientation::LeftWall);
		break;

	case ETunnelOrientation::RightWall:
		SetTunnelOrientation(ETunnelOrientation::Floor);
		break;
	}
}

//두 함수 통합, 중복 처리 방지
void ABattleCharacter::ApplySurfaceChange(const FVector& NewNormal)
{
	if (bSurfaceChanging)
	{
		return;
	}

	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (!MoveComp)
	{
		return;
	}

	const FVector CurrentUp =
		-MoveComp->GetGravityDirection().GetSafeNormal();

	const FVector NewUp =
		NewNormal.GetSafeNormal();

	const float Dot =
		FVector::DotProduct(CurrentUp, NewUp);

	// 거의 같은 면이면 무시
	if (Dot > 0.995f)
	{
		return;
	}

	const FVector Forward =
		BaseRunDirection.GetSafeNormal();

	const FVector Cross =
		FVector::CrossProduct(CurrentUp, NewUp);

	const float SinAngle =
		FVector::DotProduct(Forward, Cross);

	const float CosAngle =
		FVector::DotProduct(CurrentUp, NewUp);

	const float SignedAngle =
		FMath::RadiansToDegrees(
			FMath::Atan2(SinAngle, CosAngle)
		);

	if (FMath::Abs(SignedAngle) < 2.0f)
	{
		return;
	}

	bSurfaceChanging = true;

	RotateCameraByAngle(SignedAngle);
	SetSurfaceNormal(NewUp);
}

void ABattleCharacter::TrySetSurfaceFromHit(const FHitResult& Hit)
{
	UCharacterMovementComponent* MoveComp =
		GetCharacterMovement();

	if (!MoveComp || bSurfaceChanging)
	{
		return;
	}

	const FVector HitNormal =
		Hit.ImpactNormal.GetSafeNormal();

	const FVector CurrentUp =
		-MoveComp->GetGravityDirection().GetSafeNormal();

	const float Dot =
		FMath::Clamp(
			FVector::DotProduct(
				CurrentUp,
				HitNormal
			),
			-1.0f,
			1.0f
		);

	const float Angle =
		FMath::RadiansToDegrees(
			FMath::Acos(Dot)
		);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("SURFACE HIT | Normal=%s CurrentUp=%s Angle=%.2f"),
		*HitNormal.ToString(),
		*CurrentUp.ToString(),
		Angle
	);

	// 거의 같은 면이면 무시
	if (Angle < 10.0f)
	{
		return;
	}

	// 일단 테스트 단계에서는 100도 이하까지 허용
	if (Angle > 100.0f)
	{
		return;
	}

	ApplySurfaceChange(HitNormal);
}


void ABattleCharacter::RotateCameraByAngle(float Angle)
{
	if (bCameraRotating || !CameraRollRoot)
	{
		return;
	}

	const FRotator CurrentRotation =
		CameraRollRoot->GetRelativeRotation();

	CameraRollStart = CurrentRotation.Roll;

	// 부호는 실제 테스트 후 반대로 해야 할 수도 있음
	CameraRollTarget = CameraRollStart - Angle;

	CameraRollElapsed = 0.0f;
	bCameraRotating = true;
}

//사각형태말고 확장형 맵에 적합한 맵 회전 함수 2
//void ABattleCharacter::TrySetSurfaceFromHit(const FHitResult& Hit)
//{
//	if (bSurfaceChanging)
//	{
//		return;
//	}
//
//	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
//	if (!MoveComp)
//	{
//		return;
//	}
//
//	const FVector HitNormal =
//		Hit.ImpactNormal.GetSafeNormal();
//
//	const FVector CurrentUp =
//		-MoveComp->GetGravityDirection().GetSafeNormal();
//
//	const FVector Forward =
//		BaseRunDirection.GetSafeNormal();
//
//
//	// 1. 현재 밟고 있는 면이면 무시
//	const float UpDot =
//		FVector::DotProduct(CurrentUp, HitNormal);
//
//	if (UpDot > 0.98f)
//	{
//		return;
//	}
//
//
//	// 2. 터널 진행 방향을 바라보는 면이면 무시
//	// 타일 앞/뒤 면, 경계면 등을 막기 위함
//	const float ForwardDot =
//		FMath::Abs(
//			FVector::DotProduct(HitNormal, Forward)
//		);
//
//	if (ForwardDot > 0.3f)
//	{
//		UE_LOG(LogTemp, Warning,
//			TEXT("IGNORE FRONT/BACK | ForwardDot=%.2f"),
//			ForwardDot);
//
//		return;
//	}
//
//
//	// 3. 정말 그 면을 향해 이동하고 있었는지 확인
//	const FVector VelocityDir =
//		MoveComp->Velocity.GetSafeNormal();
//
//	//const float ApproachDot =
//	//	FVector::DotProduct(
//	//		VelocityDir,
//	//		HitNormal
//	//	);
//
//	// HitNormal 반대 방향으로 들어가고 있어야 실제 충돌
//	// 0에 가깝다면 그냥 스치거나 바닥 경계에 접촉한 것
//	//if (ApproachDot > -0.1f)
//	//{
//	//	UE_LOG(LogTemp, Warning,
//	//		TEXT("IGNORE GLANCING HIT | ApproachDot=%.2f"),
//	//		ApproachDot);
//
//	//	return;
//	//}
//
//
//	// 4. 현재 바닥 → 새 바닥의 실제 각도 계산
//	const FVector Cross =
//		FVector::CrossProduct(
//			CurrentUp,
//			HitNormal
//		);
//
//	const float SinAngle =
//		FVector::DotProduct(
//			Forward,
//			Cross
//		);
//
//	const float CosAngle =
//		FVector::DotProduct(
//			CurrentUp,
//			HitNormal
//		);
//
//	const float SignedAngle =
//		FMath::RadiansToDegrees(
//			FMath::Atan2(
//				SinAngle,
//				CosAngle
//			)
//		);
//
//
//	// 너무 미세한 Normal 변화는 무시
//	if (FMath::Abs(SignedAngle) < 5.0f)
//	{
//		return;
//	}
//
//
//	//UE_LOG(
//	//	LogTemp,
//	//	Warning,
//	//	TEXT(
//	//		"SURFACE ACCEPT | Normal=%s UpDot=%.2f ForwardDot=%.2f Approach=%.2f Angle=%.2f"
//	//	),
//	//	*HitNormal.ToString(),
//	//	UpDot,
//	//	ForwardDot,
//	//	ApproachDot,
//	//	SignedAngle
//	//);
//
//
//	bSurfaceChanging = true;
//
//	RotateCameraByAngle(SignedAngle);
//	SetSurfaceNormal(HitNormal);
//
//	GetWorldTimerManager().SetTimer(
//		SurfaceChangeTimer,
//		this,
//		&ABattleCharacter::UnlockSurfaceChange,
//		0.5f,
//		false
//	);
//}

////기존 SideDot으로 왼, 오 벽 접지 및 회전 여부 판단해서 맵 회전하는 함수1 
//void ABattleCharacter::TrySetSurfaceFromHit(const FHitResult& Hit)
//{
//
//	if (bSurfaceChanging)
//	{
//		return;
//	}
//
//	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
//	if (!MoveComp)
//	{
//		return;
//	}
//
//	const FVector HitNormal =
//		Hit.ImpactNormal.GetSafeNormal();
//
//	const FVector CurrentUp =
//		-MoveComp->GetGravityDirection().GetSafeNormal();
//
//	// 지금 밟고 있는 면이면 무시
//	const float UpDot =
//		FVector::DotProduct(CurrentUp, HitNormal);
//
//	if (UpDot > 0.9f)
//	{
//		return;
//	}
//
//	const FVector CurrentRight =
//		FVector::CrossProduct(
//			CurrentUp,
//			BaseRunDirection
//		).GetSafeNormal();
//
//	const float SideDot =
//		FVector::DotProduct(
//			HitNormal,
//			CurrentRight
//		);
//
//	UE_LOG(
//		LogTemp,
//		Warning,
//		TEXT("SURFACE HIT | Normal=%s UpDot=%.2f SideDot=%.2f"),
//		*HitNormal.ToString(),
//		UpDot,
//		SideDot
//	);
//
//	// 오른쪽 인접 면
//	if (SideDot > 0.7f)
//	{
//		bSurfaceChanging = true;
//
//		RotateCameraLeft();
//		SetSurfaceNormal(HitNormal);
//
//		GetWorldTimerManager().SetTimer(
//			SurfaceChangeTimer,
//			this,
//			&ABattleCharacter::UnlockSurfaceChange,
//			0.5f,
//			false
//		);
//	}
//
//	// 왼쪽 인접 면
//	else if (SideDot < -0.7f)
//	{
//		bSurfaceChanging = true;
//
//		RotateCameraRight();
//		SetSurfaceNormal(HitNormal);
//
//		GetWorldTimerManager().SetTimer(
//			SurfaceChangeTimer,
//			this,
//			&ABattleCharacter::UnlockSurfaceChange,
//			0.5f,
//			false
//		);
//	}
//}

void ABattleCharacter::SetSurfaceNormal(const FVector& SurfaceNormal)
{
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (!MoveComp)
	{
		return;
	}

	const FVector NewUp =
		SurfaceNormal.GetSafeNormal();

	const FVector NewGravity =
		-NewUp;

	MoveComp->SetGravityDirection(NewGravity);

	// 새 바닥에서 약간 떨어뜨림
	const float SurfaceOffset = 15.0f;

	AddActorWorldOffset(
		NewUp * SurfaceOffset,
		true
	);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("GRAVITY CHANGED | NewUp=%s NewGravity=%s"),
		*NewUp.ToString(),
		*NewGravity.ToString()
	);
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

void ABattleCharacter::RotateCameraRight()
{
	if (bCameraRotating || !CameraRollRoot)
	{
		return;
	}

	const FRotator CurrentRotation =
		CameraRollRoot->GetRelativeRotation();

	CameraRollStart = CurrentRotation.Roll;
	CameraRollTarget = CameraRollStart - 90.0f;

	CameraRollElapsed = 0.0f;
	bCameraRotating = true;
}

void ABattleCharacter::RotateCameraLeft()
{
	if (bCameraRotating || !CameraRollRoot)
	{
		return;
	}

	const FRotator CurrentRotation =
		CameraRollRoot->GetRelativeRotation();

	CameraRollStart = CurrentRotation.Roll;
	CameraRollTarget = CameraRollStart + 90.0f;

	CameraRollElapsed = 0.0f;
	bCameraRotating = true;
}

void ABattleCharacter::UpdateCameraRoll(float DeltaTime)
{
	if (!bCameraRotating || !CameraRollRoot)
	{
		return;
	}

	CameraRollElapsed += DeltaTime;

	const float Alpha =
		FMath::Clamp(
			CameraRollElapsed / CameraRollDuration,
			0.0f,
			1.0f
		);

	const float SmoothAlpha =
		FMath::InterpEaseInOut(
			0.0f,
			1.0f,
			Alpha,
			2.0f
		);

	const float NewRoll =
		FMath::Lerp(
			CameraRollStart,
			CameraRollTarget,
			SmoothAlpha
		);

	FRotator NewRotation =
		CameraRollRoot->GetRelativeRotation();

	NewRotation.Roll = NewRoll;

	CameraRollRoot->SetRelativeRotation(NewRotation);

	if (Alpha >= 1.0f)
	{
		NewRotation.Roll = CameraRollTarget;
		CameraRollRoot->SetRelativeRotation(NewRotation);

		bCameraRotating = false;
		bSurfaceChanging = false;

		if (bOnWall)
		{
			RequestWallOff();
		}
	}
}

void ABattleCharacter::RestoreTunnelGravity()
{
	SetTunnelOrientation(CurrentTunnelOrientation);
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

void ABattleCharacter::StartSlide()
{
	UE_LOG(LogTemp, Warning, TEXT("START SLIDE"));
	if (bIsSliding)
		return;

	bIsSliding = true;

	// 캡슐 높이 축소
	Crouch();
	
}

void ABattleCharacter::EndSlide()
{
	UE_LOG(LogTemp, Warning, TEXT("End SLIDE"));
	if (!bIsSliding)
		return;

	bIsSliding = false;

	// 원래 캡슐 높이로 복구
	UnCrouch();

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
	if (bMapRotating)
	{
		return;
	}

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
		//기존 Gravity 방향
		//const FVector RightDirection =
		//	FVector::CrossProduct(
		//		FVector::UpVector,
		//		BaseRunDirection
		//	).GetSafeNormal();

		//Graviy 기준을 수정한 벽(바닥) 방향으로 변경
		const FVector CurrentUp =
			-MoveComp->GetGravityDirection();

		const FVector RightDirection =
			FVector::CrossProduct(
				CurrentUp,
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
	//if (!bOnWall)
	//{
	//	const FRotator TargetRotation =
	//		BaseRunDirection.Rotation();

	//	const FRotator NewRotation =
	//		FMath::RInterpTo(
	//			GetActorRotation(),
	//			TargetRotation,
	//			DeltaTime,
	//			10.0f
	//		);

	//	SetActorRotation(NewRotation);
	//}
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
		else if ((WallSpike || Spike) && Cast<UStaticMeshComponent>(OtherComp)) {
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
		GetCharacterMovement()->StopMovementImmediately();
	}
}

void ABattleCharacter::Tick(float DeltaTime) {

	Super::Tick(DeltaTime);

	if (!CanMove)
	{
		return;
	}
	//매 초 밟고 있는 바닥 갱신
	//UpdateSurfaceFromFloor();



	//AddMovementInput(MoveDirection, 1.0f);
	UpdateRunnerMovement(DeltaTime);

	//if (bChaserActive) {
	//	UpdateChaserPosition();
	//}

	UpdateCameraRoll(DeltaTime);

	const UCharacterMovementComponent* Movement = GetCharacterMovement();

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("Mode=%d Gravity=%s Velocity=%s Location=%s"),
		static_cast<int32>(Movement->MovementMode),
		*Movement->GetGravityDirection().ToString(),
		*Movement->Velocity.ToString(),
		*GetActorLocation().ToString()
	);

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Control=%s | BoomTarget=%s | CameraWorld=%s"),
			*PC->GetControlRotation().ToString(),
			*CameraBoom->GetTargetRotation().ToString(),
			*FollowCamera->GetComponentRotation().ToString()
		);
	}
	


	

}