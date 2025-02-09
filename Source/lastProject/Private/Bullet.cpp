#include "Bullet.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Boss.h"
#include "BossFSM.h"

// Sets default values
ABullet::ABullet()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	collisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
	collisionComp->SetCollisionProfileName(TEXT("BlockAll"));
	collisionComp->SetSphereRadius(13);
	RootComponent = collisionComp;

	bodyMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMeshComp"));
	bodyMeshComp->SetupAttachment(collisionComp);
	bodyMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	bodyMeshComp->SetRelativeScale3D(FVector(0.25f));

	movementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("MovementComp"));
	movementComp->SetUpdatedComponent(collisionComp);
	movementComp->InitialSpeed = 5000;
	movementComp->MaxSpeed = 5000;
	movementComp->bShouldBounce = false;
	movementComp->Bounciness = 0.3f;

	InitialLifeSpan = 2.0f;

	collisionComp->SetNotifyRigidBodyCollision(true); // 충돌 알림 활성화

}

// Called when the game starts or when spawned
void ABullet::BeginPlay()
{
	Super::BeginPlay();

	if (collisionComp)
	{
		collisionComp->OnComponentHit.AddDynamic(this, &ABullet::OnHit);
	}

}

// Called every frame
void ABullet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	UE_LOG(LogTemp, Log, TEXT("Bullet hit: %s"), *OtherActor->GetName());

	if (OtherActor && OtherActor->IsA(ABoss::StaticClass())) // 충돌한 대상이 Boss인 경우
	{
		ABoss* HitBoss = Cast<ABoss>(OtherActor);
		if (HitBoss)
		{
			// Boss FSM 가져와서 데미지 처리 호출
			UBossFSM* BossFSM = Cast<UBossFSM>(HitBoss->GetComponentByClass(UBossFSM::StaticClass()));
			if (BossFSM)
			{
				float DamageAmount = 2.0f; // 데미지 양 (원하는 값으로 설정)
				BossFSM->OnDamageProcess(DamageAmount);
			}
		}
	}

	// 총알 제거
	Destroy();
}

