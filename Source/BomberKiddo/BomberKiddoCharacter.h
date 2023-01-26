// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BomberKiddoCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerReceiveDamage, int, CurrentLife);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerDeath);  // old single player
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerLost);  // new multiplayer
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPlayerWon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FThePlayerWon); 

class UParticleSystemComponent;
class ABK_Bomb;
class USpringArmComponent;
class UCameraComponent;

UCLASS(config = Game)
class ABomberKiddoCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		UCameraComponent* FollowCamera;

public:
	ABomberKiddoCharacter();

	UPROPERTY(BlueprintAssignable)
		FOnPlayerReceiveDamage OnPlayerReceiveDamage;

	UPROPERTY(BlueprintAssignable)
		FOnPlayerDeath OnPlayerDeath;

	UPROPERTY(BlueprintAssignable)
		FOnPlayerLost OnPlayerLost;

	UPROPERTY(BlueprintAssignable)
		FPlayerWon PlayerWon;

	UPROPERTY()
		FThePlayerWon ThePlayerHasWon;
	
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Camera)
		float BaseLookUpRate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UParticleSystemComponent* ExplosionParticle = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		USceneComponent* SpawnBombPoint = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		USceneComponent* BigBombSpawnPoint = nullptr;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
		UAnimMontage* ThrowBombAnim;

	//UPROPERTY(ReplicatedUsing = OnRep_bPlayerDead)
	UPROPERTY(Replicated)
		bool bPlayerDead = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int CurrentLife;

	UPROPERTY()
		class UHorizontalBox* PlayerInfo;

	UPROPERTY()
		class UTextBlock* ChoosenLifeAmount;

	UFUNCTION(Client, Reliable)
		void ClientPlayThrowBombAnim(bool bSmallBomb);

	UFUNCTION(Server, Reliable)
		void ServerThrowBombAnim();

	UFUNCTION(NetMulticast, Reliable)
		void MultiCastPlayThrowBombAnim();

	UFUNCTION(blueprintImplementableEvent)
		void BP_OnLost();

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
		TSubclassOf<ABK_Bomb> ServerSmallBomb;

	UFUNCTION(BlueprintCallable)
		void GetDamage(int Damage);

	UFUNCTION(BlueprintCallable)
		FORCEINLINE int GetCurrentLife() { return CurrentLife; }; 

	UFUNCTION(BlueprintCallable)
		FORCEINLINE int GetMaxLife() { return MaxLife; };

	UFUNCTION(BlueprintCallable)
		void SetMaxLife(int StartingLife);

	UFUNCTION(BlueprintCallable)
		bool GetCanThrowBomb();

	void StopMovement();

	UFUNCTION(BlueprintImplementableEvent)
		void BP_GetDamageEvent(int ActualCurrentLife);

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int MaxLife;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float SmallBombShootCooldown = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float BigBombShootCooldown = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float BombStartTime = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float DeathSoundStartTime = 0.5f;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
		TSubclassOf<ABK_Bomb> SmallBomb;

	//UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
		//TSubclassOf<ABK_Bomb> ServerBomb;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSubclassOf<ABK_Bomb> BigBomb;

	UFUNCTION(BlueprintCallable)
		void ThrowBomb();

	UFUNCTION(BlueprintCallable)
		void StartThrowBomb();

	UFUNCTION(BlueprintCallable)
		void ThrowBigBomb();

	float InViewPitchMin = -20.f;
	float InViewPitchMax = 20.f;

	bool bCanThrowBomb = true;
	bool bCanThrowBigBomb = true;

	FTimerHandle SmallBombCooldownTimeHandle;
	FTimerHandle BigBombCooldownTimeHandle;

	// Cooldown Reset
	void ResetCanThrowBomb();
	void ResetCanThrowBigBomb();

	// SFX Functions:
	void PlayLaunchBombSound();
	void PlayDeathSound();

	// Limit Camera View Pitch
	APlayerCameraManager* CameraManager;

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

	virtual void BeginPlay() override;

	FTimerHandle TimerHandle;
	FTimerDelegate TimerDelegate;

	FTimerHandle PlayDeathSoundHandle;


public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

private:

	UPROPERTY(EditAnywhere, Category = "Combat")
		class USoundBase* LaunchBombSound;

	UPROPERTY(EditAnywhere, Category = "Combat")
		class USoundBase* DeathSound;

	UWorld* CheckIfCanThrow();

	UWorld* TheWorld = nullptr;
};
