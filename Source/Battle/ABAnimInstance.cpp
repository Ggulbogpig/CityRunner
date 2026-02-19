// Fill out your copyright notice in the Description page of Project Settings.


#include "ABAnimInstance.h"

UABAnimInstance::UABAnimInstance() {
	CurrentPawnSpeed = 0.0f;
	IsInAir = false;
	static ConstructorHelpers::FObjectFinder<UAnimMontage> ATTACK_MONTAGE(TEXT("/Script/Engine.AnimMontage'/Game/Anim/Exo_Gray_Skeleton_Montage1.Exo_Gray_Skeleton_Montage1'"));
	if (ATTACK_MONTAGE.Succeeded()) {
		AttackMontage = ATTACK_MONTAGE.Object;
	}
}

void UABAnimInstance::NativeUpdateAnimation(float DeltaSeconds) {
	Super::NativeUpdateAnimation(DeltaSeconds);

	auto Pawn = TryGetPawnOwner();
	if (::IsValid(Pawn)) {
		CurrentPawnSpeed = Pawn->GetVelocity().Size(); //속도의 크기인 속력만 갖고오기
		//점프 설정
		auto Character = Cast<ACharacter>(Pawn);
		if (Character) {
			IsInAir = Character->GetMovementComponent()->IsFalling();
		}
	}
}

void UABAnimInstance::PlayAttackMontage() {
	
		Montage_Play(AttackMontage, 1.0f);
	
}

void UABAnimInstance::JumpToAttackMontageSection(int32 NewSection) {
	ABCHECK(Montage_IsPlaying(AttackMontage));
	Montage_JumpToSection(GetAttackMontageSectionName(NewSection), AttackMontage);
}

void UABAnimInstance::AnimNotify_AttackHitCheck() {
	OnAttackHitCheck.Broadcast();
}

void UABAnimInstance::AnimNotify_NextAttackCheck() {
	OnNextAttackCheck.Broadcast();
}

FName UABAnimInstance::GetAttackMontageSectionName(int32 Section) {
	ABCHECK(FMath::IsWithinInclusive<int32>(Section, 1, 4), NAME_None);
	return FName(*FString::Printf(TEXT("Attack%d"), Section));
}

