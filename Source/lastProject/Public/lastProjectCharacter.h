// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "lastProjectCharacter.generated.h"

UCLASS(Blueprintable)
class AlastProjectCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AlastProjectCharacter();

	// Called every frame.
	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay();

	/** Returns TopDownCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** HP 변수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CharacterStats)
	float CurrentHP;

	/** 최대 HP */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = CharacterStats)
	float MaxHP;

	/** 데미지 처리 함수 */
	UFUNCTION(BlueprintCallable, Category = CharacterStats)
	void TakeDamage(float DamageAmount);

	bool isDie;

	UPROPERTY(EditDefaultsOnly, Category = BulletFactory)
	TSubclassOf<class ABullet> bulletFactory;

	UPROPERTY(VisibleAnywhere, Category = GunMesh)
	class UStaticMeshComponent* DefaultGunComp;
	UPROPERTY(VisibleAnywhere, Category = GunMesh)
	class UStaticMeshComponent* GatlingGunComp;
	UPROPERTY(VisibleAnywhere, Category = GunMesh)
	class UStaticMeshComponent* sniperGunComp;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> DeathWidgetClass;



	UPROPERTY(EditDefaultsOnly, Category = Cooldowns)
	float GatlingCooldownTime;

	float GatlingElapsedTime = 0.0f;

	FTimerHandle GatlingCooldownHandle;

	bool bCanUseGatling;

	void Fire();
	void StartGatlingGun();
	void StopGatlingGun();
	void Die();

protected:
	// 발사 타이머 핸들
	FTimerHandle FireTimerHandle;

	// 총알 발사 위치 소켓 이름 배열
	TArray<FName> GatlingGunSocketNames;

	// 총알 발사 간격 (초 단위)
	float FireInterval = 0.1f;

	// 발사 지속 시간 (5초)
	float FireDuration = 5.0f;

	// 게틀링건 발사 함수
	void FireGatlingGun();

	// 발사 종료 함수

private:
	/** Top down camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* TopDownCameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;




	

};

