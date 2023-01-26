// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BK_Cube.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCubeReceiveDamage, int, CurrentLife);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCubeDeath);

class UBoxComponent;
class UStaticMeshComponent;
class UParticleSystemComponent;

UCLASS()
class BOMBERKIDDO_API ABK_Cube : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABK_Cube();

	UPROPERTY(BlueprintAssignable)
		FOnCubeReceiveDamage OnCubeReceiveDamage;

	UPROPERTY(BlueprintAssignable)
		FOnCubeDeath OnCubeDeath;

	UFUNCTION(BlueprintCallable)
		void GetDamage(int Damage);

	UFUNCTION()
		void HandleDestruction();

	UFUNCTION(BlueprintCallable)
		FORCEINLINE int GetCurrentLife() { return CurrentLife; };

	UFUNCTION(BlueprintCallable)
		FORCEINLINE int GetMaxLife() { return MaxLife; };

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UBoxComponent* BoxCollision = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UStaticMeshComponent* Mesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UParticleSystemComponent* ExplosionParticle = nullptr;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int MaxLife = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int CurrentLife;

	UFUNCTION(BlueprintImplementableEvent) 
		void BP_GetDamageEvent(int ActualCurrentLife); 

	FTimerHandle TimerHandle;
	FTimerDelegate TimerDelegate;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	class ABomberKiddoGameMode* BomberKiddoGameMode;

	UPROPERTY(EditAnywhere, Category = "Movement Platform")
		FVector PlatformVelocity = FVector(100, 0, 0);

	UPROPERTY(EditAnywhere, Category = "Movement Platform")
		float MoveDistance = 100;

	UPROPERTY(EditAnywhere, Category = "Rotation")
		FRotator RotationVelocity;

	UPROPERTY(EditAnywhere, Category = "Combat")
		class UParticleSystem* DeathParticles;

	UPROPERTY(EditAnywhere, Category = "Combat")
		class USoundBase* DeathSound;

	FVector StartLocation;

	void MovePlatform(float DeltaTime);
	void RotatePlatform(float DeltaTime);

	bool ShouldPlatformReturn() const;
	float GetDistanceMoved() const;

};