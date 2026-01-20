// Fill out your copyright notice in the Description page of Project Settings.


#include "ABPlayerController.h"

void AABPlayerController::PostInitializeComponents() {
	Super::PostInitializeComponents();
	ABLOG_S(Warning);
}

void AABPlayerController::OnPossess(APawn* aPawn) {
	ABLOG_S(Warning);
	Super::OnPossess(aPawn);
}

void AABPlayerController::BeginPlay() {
	Super::BeginPlay();

	FInputModeGameOnly InputMode; //UI배제하고 바로 게임에게만 입력을 전달하도록 함. 뷰포트를 클릭해서 포커스를 잡을 필요가 없어짐. 
	SetInputMode(InputMode);
}
