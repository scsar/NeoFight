// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Mine.generated.h"

UCLASS()
class LASTPROJECT_API AMine : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category = Mesh)
	class UStaticMeshComponent* MineMesh;

	UPROPERTY(VisibleAnywhere, Category = Collision)
	class UBoxComponent* collisionComp;

	FTimerHandle ExplosionTimerHandle;

	UPROPERTY(EditDefaultsOnly, Category = Effects)
	class UParticleSystem* ExplosionEffect;

	UPROPERTY(EditDefaultsOnly, Category = Settings)
	float TimeToExplode = 10.0f;

	bool bHasPawnContact;

	UPROPERTY(EditAnywhere, Category = Sound)
	USoundBase* MineSound;
	
public:	
	// Sets default values for this actor's properties
	AMine();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void Explode();

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

};
