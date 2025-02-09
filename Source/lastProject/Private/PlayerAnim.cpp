// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerAnim.h"
#include "lastProjectCharacter.h"
#include "lastProjectPlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/MovementComponent.h"

void UPlayerAnim::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    // Owning Actor ��������
    APawn* OwningPawn = TryGetPawnOwner();
    if (OwningPawn)
    {
        // ĳ���ͷ� ĳ����
        Character = Cast<AlastProjectCharacter>(OwningPawn);
        if (Character)
        {
            // Movement Component ��������
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
        // ���� ��ġ�� ��ǥ ��ġ ���� �Ÿ� ���
        FVector CurrentLocation = Character->GetActorLocation();
        float DistanceToTarget = FVector::Dist(CurrentLocation, CachedDestination);

        // �Ÿ� ���� ��� (0.0 ~ 1.0)
        float DistanceRatio = FMath::Clamp(DistanceToTarget / MaxDistance, 0.0f, 1.0f);

        // ���� ���� ���� (���� ����)
        FVector NewAcceleration = Character->GetCharacterMovement()->GetCurrentAcceleration();


        // ������ ��ȿ�� ���ӵ� �� ����
        if (!NewAcceleration.IsNearlyZero())
        {
            LastValidAcceleration = NewAcceleration; // ������ ��ȿ �� ������Ʈ
        }

        // ��ȿ�� �� �Ǵ� ������ ���� ���
        CurrentAcceleration = LastValidAcceleration * DistanceRatio;

        // �ּ� ���ӵ� ����
        if (DistanceToTarget <= 120.0f) // ���� �Ÿ� ����
        {
            CurrentAcceleration = FVector::ZeroVector;
            LastValidAcceleration = FVector::ZeroVector;
            speed = 0;
        }

        

    }
}
