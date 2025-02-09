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

	/** HP ���� */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CharacterStats)
	float CurrentHP;

	/** �ִ� HP */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = CharacterStats)
	float MaxHP;

	/** ������ ó�� �Լ� */
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
	// �߻� Ÿ�̸� �ڵ�
	FTimerHandle FireTimerHandle;

	// �Ѿ� �߻� ��ġ ���� �̸� �迭
	TArray<FName> GatlingGunSocketNames;

	// �Ѿ� �߻� ���� (�� ����)
	float FireInterval = 0.1f;

	// �߻� ���� �ð� (5��)
	float FireDuration = 5.0f;

	// ��Ʋ���� �߻� �Լ�
	void FireGatlingGun();

	// �߻� ���� �Լ�

private:
	/** Top down camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* TopDownCameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;




	

};

