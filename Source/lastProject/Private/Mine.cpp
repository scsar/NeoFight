// Fill out your copyright notice in the Description page of Project Settings.


#include "Mine.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "lastProjectPlayerController.h"
#include "TimerManager.h"
#include "Boss.h"
#include "BossFSM.h"

// Sets default values
AMine::AMine()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	collisionComp = CreateDefaultSubobject<UBoxComponent>(TEXT("collisionComp"));
	RootComponent = collisionComp;

	collisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	collisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	collisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	MineMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MineMesh"));
	MineMesh->SetupAttachment(collisionComp);

	collisionComp->OnComponentBeginOverlap.AddDynamic(this, &AMine::OnOverlapBegin);
	collisionComp->OnComponentEndOverlap.AddDynamic(this, &AMine::OnOverlapEnd);

	// �ʱⰪ
	bHasPawnContact = false;

	

}

// Called when the game starts or when spawned
void AMine::BeginPlay()
{
	Super::BeginPlay();
	
	GetWorldTimerManager().SetTimer(ExplosionTimerHandle, this, &AMine::Explode, TimeToExplode, false);

	DrawDebugBox(GetWorld(), GetActorLocation(), collisionComp->GetScaledBoxExtent(),
		FQuat::Identity, FColor::Red, false, 5.0f, 0, 5.0f);
}

// Called every frame
void AMine::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMine::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor->IsA(ABoss::StaticClass())) // �浹�� ����� Boss�� ���
	{
		bHasPawnContact = true;
		ABoss* HitBoss = Cast<ABoss>(OtherActor);
		if (HitBoss)
		{
			// Boss FSM �����ͼ� ������ ó�� ȣ��
			UBossFSM* BossFSM = Cast<UBossFSM>(HitBoss->GetComponentByClass(UBossFSM::StaticClass()));
			if (BossFSM)
			{
				float DamageAmount = 10.0f; // ������ �� (���ϴ� ������ ����)
				BossFSM->OnDamageProcess(DamageAmount);
			}
			Explode();
		}
	}
	
}

void AMine::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

	if (OtherActor && OtherActor->IsA(ABoss::StaticClass())) // �浹�� ����� Boss�� ���
	{
		bHasPawnContact = false;
	}
	
}

void AMine::Explode()
{
	// Ÿ�̸� ����
	GetWorldTimerManager().ClearTimer(ExplosionTimerHandle);

	// ���� ����Ʈ ����
	if (ExplosionEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
	}
	// �������� ���� ���
	if (MineSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, MineSound, GetActorLocation());
	}

	// ����� �α�
	UE_LOG(LogTemp, Warning, TEXT("Mine exploded!"));

	// ���� �ð� �� ����
	Destroy();
}
