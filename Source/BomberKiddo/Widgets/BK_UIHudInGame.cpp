// Fill out your copyright notice in the Description page of Project Settings.


#include "BK_UIHudInGame.h"
#include "../BomberKiddoCharacter.h" 
#include "../BomberKiddoGameMode.h" 
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/VerticalBox.h"
#include "Components/EditableTextBox.h"
#include "Components/Overlay.h"
#include "Kismet/GameplayStatics.h"

void UBK_UIHudInGame::NativeOnInitialized()
{
	Super::OnInitialized();

	Character = UGameplayStatics::GetPlayerCharacter(this, 0);
	PController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	GameMode = UGameplayStatics::GetGameMode(this);
	
	if (ABomberKiddoCharacter* BK_Character = Character ? Cast<ABomberKiddoCharacter>(Character) : nullptr)
	{
		//BK_Character->OnPlayerReceiveDamage.AddDynamic(this, &UBK_UIHudInGame::OnPlayerReceiveDamage);
		//BK_Character->OnPlayerDeath.AddDynamic(this, &UBK_UIHudInGame::OnPlayerDeath);
		BK_Character->OnPlayerLost.AddDynamic(this, &UBK_UIHudInGame::PlayerLost);
		//BK_Character->PlayerWon.AddDynamic(this, &UBK_UIHudInGame::ThePlayerWon);
	}

	if (ABomberKiddoGameMode* BK_GameMode = GameMode ? Cast<ABomberKiddoGameMode>(GameMode) : nullptr && !PlayerDead)
	{
		BK_GameMode->OnPlayerWin.AddDynamic(this, &UBK_UIHudInGame::OnPlayerWin);
	}

	// Setting Actual Widget Texts
	FName InfoTextName = FName(TEXT("TextControl"));
	FName LifeConstraintTextName = FName(TEXT("LifeAmountConstraint"));
	FName NumberConstraintTextName = FName(TEXT("NotNumberConstraint"));
	FName OverlayTextName = FName(TEXT("InsertLife"));
	FName DeathScreenTextName = FName(TEXT("DeathScreen"));
	FName WonScreenTextName = FName(TEXT("WonScreen"));
	FName PlayerInfoName = FName(TEXT("PlayerLifeInfo"));
	FName LifeTextName = FName(TEXT("LifeText"));
	FName TimerTextName = FName(TEXT("Timer"));
	FName ButtonsTextName = FName(TEXT("DeathButtons"));

	// Setting Widget Elements
	InfoText = (UTextBlock*)(WidgetTree->FindWidget(InfoTextName));
	ConstraintText = (UTextBlock*)(WidgetTree->FindWidget(LifeConstraintTextName));
	NumberConstraintText = (UTextBlock*)(WidgetTree->FindWidget(NumberConstraintTextName));
	ChoosenLifeAmount = (UTextBlock*)(WidgetTree->FindWidget(LifeTextName));
	HudTimer = (UTextBlock*)(WidgetTree->FindWidget(TimerTextName));
	HudOverlay = (UOverlay*)(WidgetTree->FindWidget(OverlayTextName));
	WonOverlay = (UOverlay*)(WidgetTree->FindWidget(WonScreenTextName));
	DeathOverlay = (UOverlay*)(WidgetTree->FindWidget(DeathScreenTextName));
	PlayerInfo = (UHorizontalBox*)(WidgetTree->FindWidget(PlayerInfoName));
	DeathButtons = (UVerticalBox*)(WidgetTree->FindWidget(ButtonsTextName));
	
	if (InfoText != nullptr && ChoosenLifeAmount != nullptr)
	{
		InfoText->SetText(FText::FromString("To start playing please:"));
		ChoosenLifeAmount->SetText(FText::FromString("x/x "));
	}

	// Setting EditableBox
	FName LifeBoxTextName = FName(TEXT("EditableText"));

	if (LifeBox == nullptr)
	{
		LifeBox = (UEditableTextBox*)(WidgetTree->FindWidget(LifeBoxTextName));
	}

	if (LifeBox != nullptr)
	{
		LifeBox->OnTextCommitted.AddDynamic(this, &UBK_UIHudInGame::DelegateCommitInputText);
	}

}

void UBK_UIHudInGame::OnPlayerReceiveDamage(int CurrentLife)
{
	ChoosenLifeAmount->SetText(FText::FromString(FString::FromInt(CurrentLife) + "/" + *LifeAmountChoosen));	
}

void UBK_UIHudInGame::OnPlayerWin()
{
	PlayerWon = true;
	PlayerInfo->SetVisibility(ESlateVisibility::Collapsed);
	WonOverlay->SetVisibility(ESlateVisibility::HitTestInvisible);
	DeathButtons->SetVisibility(ESlateVisibility::Visible);
	PController->SetInputMode(FInputModeUIOnly());
	BP_OnPlayerWin();
	PlayWinSound();
}

void UBK_UIHudInGame::PlayerLost()
{
	BP_OnPlayerLost();
}

void UBK_UIHudInGame::OnPlayerDeath()
{
	PlayerDead = true;
	PlayerInfo->SetVisibility(ESlateVisibility::Collapsed);
	DeathOverlay->SetVisibility(ESlateVisibility::HitTestInvisible);
	DeathButtons->SetVisibility(ESlateVisibility::Visible);
	ChoosenLifeAmount->SetText(FText::FromString(FString::FromInt(0) + "/" + *LifeAmountChoosen));
	PController->SetInputMode(FInputModeUIOnly());
	Cast<ABomberKiddoCharacter>(Character)->StopMovement();
	Cast<ABomberKiddoCharacter>(Character)->BP_GetDamageEvent(0);
	BP_OnPlayerDeath();
	PlayLoseSound();
}

void UBK_UIHudInGame::DelegateCommitInputText(const FText& InText, ETextCommit::Type InCommitType)
{
	if (InCommitType == ETextCommit::OnEnter)
	{
		LifeAmountChoosen = InText.ToString();
		if (LifeAmountChoosen.IsNumeric())
		{
			int32 IntLifeChoosen = FCString::Atoi(*LifeAmountChoosen);
	
			if (IntLifeChoosen <= 10 && IntLifeChoosen != 0)
			{
				// Setear la vida actual y maxima del player
				Cast<ABomberKiddoCharacter>(Character)->SetMaxLife(IntLifeChoosen);

				// Habilitar controles
				PController->SetInputMode(FInputModeGameOnly());

				// Borrar contenedor de overlay
				HudOverlay->SetVisibility(ESlateVisibility::Collapsed);
								
				// Setear el total de vida
				ChoosenLifeAmount->SetText(FText::FromString(FString::FromInt(IntLifeChoosen) + "/" + FString::FromInt(IntLifeChoosen)));
			
				// Iniciar el Timer del Juego
				GameTimer();
			}
			else 
			{
				ConstraintText->SetVisibility(ESlateVisibility::Visible);
				NumberConstraintText->SetVisibility(ESlateVisibility::Collapsed);
				// UE_LOG(LogTemp, Warning, TEXT("Number must from 1-10"));
			}
		}
		else
		{
			ConstraintText->SetVisibility(ESlateVisibility::Collapsed);
			NumberConstraintText->SetVisibility(ESlateVisibility::Visible);
			// UE_LOG(LogTemp, Warning, TEXT("Its NOT Numeric %s"), *LifeAmountChoosen);
		}
	}

	RepeatingCallsRemaining = TimerCount;
}

void UBK_UIHudInGame::GameTimer()
{
	FTimerDelegate GameOverTimerDelegate = FTimerDelegate::CreateUObject(
		this,												   // Clase a utilizar
		&UBK_UIHudInGame::OnPlayerDeath                        // Funcion de la clase
														       // Parametro pasado a la clase (si se quiere)
	);
	
	// Start Repeating function
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UBK_UIHudInGame::RepeatingVisualTimer, 1.0f, true, 0.f);
	
	// Start GameOverTimer
	GetWorld()->GetTimerManager().SetTimer(
		GameOverTimerHandle,
		GameOverTimerDelegate,                           
		StartDelay,
		false
	);

}

void UBK_UIHudInGame::RepeatingVisualTimer()
{

	if (--RepeatingCallsRemaining <= 0 || Cast<ABomberKiddoCharacter>(Character)->GetCurrentLife() == 0 || PlayerWon ||  PlayerDead)
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	}
	
	TimerCount--;

	// Setear el timer al Texto
	HudTimer->SetText(FText::FromString(FString::FromInt(TimerCount)));

}

void UBK_UIHudInGame::PlayWinSound()
{
	if (WinSound)
	{
		UGameplayStatics::PlaySound2D(this, WinSound);
	}
}

void UBK_UIHudInGame::PlayLoseSound()
{
	if (LoseSound)
	{
		UGameplayStatics::PlaySound2D(this, LoseSound);
	}
}


