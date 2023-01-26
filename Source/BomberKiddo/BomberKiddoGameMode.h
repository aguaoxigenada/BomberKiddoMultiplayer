// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BomberKiddoGameMode.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerWin);

UCLASS(minimalapi)
class ABomberKiddoGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ABomberKiddoGameMode();

	void ActorDied(AActor* DeadActor);

	UPROPERTY(BlueprintAssignable)
		FOnPlayerWin OnPlayerWin;

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintImplementableEvent)
		void BP_SpawnNewBrickWave(int waveNum);

private:

	class ABK_Cube* Cube;
	class ABomberKiddoPlayerController* BomberKiddoPlayerControler;
	class UBK_UIHudInGame* HudInGame;

	void HandleGameStart();

	int32 TargetCubes = 0;
	int32 GetTargetCubeCount(int actualWave);

};