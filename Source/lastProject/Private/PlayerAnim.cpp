// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerAnim.h"
#include "lastProjectCharacter.h"
#include "lastProjectPlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/MovementComponent.h"

void UPlayerAnim::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    // Owning Actor 가져오기
    APawn* OwningPawn = TryGetPawnOwner();
    if (OwningPawn)
    {
        // 캐릭터로 캐스팅
        Character = Cast<AlastProjectCharacter>(OwningPawn);
        if (Character)
        {
            // Movement Component 가져오기
            MovementComponent = Character->GetCharacterMovement();
        }
    }
    
}


void UPlayerAnim::NativeUpdateAnimation(float DeltaSeconds)
{
    if (Character)
    {
        BaseController = Character->GetController();
        AlastProjectPlayerController* playerController = Cast<AlastProjectPlayerController>(BaseController);
        if (playerController != nullptr)
        {
            CachedDestination = playerController->CachedDestination;
        }
        // 현재 위치와 목표 위치 간의 거리 계산
        FVector CurrentLocation = Character->GetActorLocation();
        float DistanceToTarget = FVector::Dist(CurrentLocation, CachedDestination);

        // 거리 비율 계산 (0.0 ~ 1.0)
        float DistanceRatio = FMath::Clamp(DistanceToTarget / MaxDistance, 0.0f, 1.0f);

        // 감속 공식 적용 (선형 감소)
        FVector NewAcceleration = Character->GetCharacterMovement()->GetCurrentAcceleration();


        // 마지막 유효한 가속도 값 저장
        if (!NewAcceleration.IsNearlyZero())
        {
            LastValidAcceleration = NewAcceleration; // 마지막 유효 값 업데이트
        }

        // 유효한 값 또는 유지된 값을 사용
        CurrentAcceleration = LastValidAcceleration * DistanceRatio;

        // 최소 가속도 설정
        if (DistanceToTarget <= 120.0f) // 도착 거리 기준
        {
            CurrentAcceleration = FVector::ZeroVector;
            LastValidAcceleration = FVector::ZeroVector;
            speed = 0;
        }

        

    }
}
