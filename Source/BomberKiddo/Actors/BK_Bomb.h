// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BK_Bomb.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UParticleSystemComponent;


UCLASS()
class BOMBERKIDDO_API ABK_Bomb : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABK_Bomb();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float ExplodeTime = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float ExplosionTime = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float Radius = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float InitialForce = 9000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		USphereComponent* SphereCollision = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UStaticMeshComponent* Mesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UParticleSystemComponent* ExplosionParticle = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UParticleSystemComponent* BombSparkParticle = nullptr;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
		void OnExplode();
	UFUNCTION()
		void OnFinishedExplosion();
	UFUNCTION()
		void OnParticleSystemFinished(UParticleSystemComponent* PSystem);
	UFUNCTION()
		void OnComponentBeginOverlapInBomb(UPrimitiveComponent* OnComponentBeginOverlap, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(BlueprintImplementableEvent)
		void BP_OnPlayerDetected(ABomberKiddoCharacter* Character);

	UFUNCTION(BlueprintImplementableEvent)
		void BP_OnBlockDetected(ABK_Cube* Block);

	UPROPERTY()
		ABomberKiddoCharacter* PlayerDamaged = nullptr;

	UPROPERTY()
		ABK_Cube* CubeDamaged = nullptr;

private:
	FTimerHandle ExplodeTimeHandle;
	FTimerHandle ExplosionTimeHandle;

	UPROPERTY(EditAnywhere, Category = "Combat")
		class USoundBase* DeathSound;

	UPROPERTY(EditAnywhere, Category = "Combat")
		float DamageOfBombType;

};