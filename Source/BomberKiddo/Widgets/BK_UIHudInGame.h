// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "BK_UIHudInGame.generated.h"


UCLASS()
class BOMBERKIDDO_API UBK_UIHudInGame : public UUserWidget
{
	GENERATED_BODY()

public:
	void NativeOnInitialized() override;

protected:
	UFUNCTION()
		void OnPlayerReceiveDamage(int CurrentLife);

	UFUNCTION()
		void OnPlayerWin();

	UFUNCTION()
		void OnPlayerDeath();

	UFUNCTION()
		void PlayerLost();

		UFUNCTION(blueprintImplementableEvent)
		void BP_OnPlayerDeath();

	UFUNCTION(blueprintImplementableEvent)
		void BP_OnPlayerLost();

	UFUNCTION(blueprintImplementableEvent)
		void BP_OnPlayerWin();

	UFUNCTION()
		void RepeatingVisualTimer();

	void GameTimer();

	void PlayWinSound();

	void PlayLoseSound();

	UFUNCTION()
		void DelegateCommitInputText(const FText& InText, ETextCommit::Type InCommitType);

	UPROPERTY()
		class UTextBlock* InfoText;

	UPROPERTY()
		UTextBlock* ConstraintText;

	UPROPERTY()
		UTextBlock* NumberConstraintText;

	UPROPERTY()
		UTextBlock* ChoosenLifeAmount;

	UPROPERTY()
		UTextBlock* HudTimer;

	UPROPERTY()
		class UOverlay* HudOverlay;

	UPROPERTY()
		UOverlay* WonOverlay;

	UPROPERTY()
		UOverlay* DeathOverlay;

	UPROPERTY()
		class UHorizontalBox* PlayerInfo;

	UPROPERTY()
		class UVerticalBox* DeathButtons;

	UPROPERTY()
		class UEditableTextBox* LifeBox;

	UPROPERTY()
		ACharacter* Character;

	UPROPERTY()
		APlayerController* PController;

	UPROPERTY()
		AGameModeBase* GameMode;

	FString LifeAmountChoosen;

private:

	float StartDelay = 240.f;
	int TimerCount = 240;
	int RepeatingCallsRemaining;
	bool PlayerDead = false;
	bool PlayerWon = false;

	FTimerHandle GameOverTimerHandle;
	FTimerHandle TimerHandle;

	UPROPERTY(EditAnywhere, Category = "Combat")
		class USoundBase* WinSound;

	UPROPERTY(EditAnywhere, Category = "Combat")
		class USoundBase* LoseSound;
};
