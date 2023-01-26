// Copyright Epic Games, Inc. All Rights Reserved.

#include "BomberKiddoGameMode.h"
#include "BomberKiddoCharacter.h"
#include "Actors/BK_Cube.h"
#include "Widgets/BK_UIHudInGame.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"


ABomberKiddoGameMode::ABomberKiddoGameMode()
{
    // set default pawn class to our Blueprinted character
    static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/Pawn/BP_BomberKiddoCharacter"));

    if (PlayerPawnBPClass.Class != NULL)
    {
        DefaultPawnClass = PlayerPawnBPClass.Class;
    }
}


void ABomberKiddoGameMode::ActorDied(AActor* DeadActor)
{
    if (ABK_Cube* DestroyedCube = Cast<ABK_Cube>(DeadActor))
    {
        DestroyedCube->HandleDestruction();
        TargetCubes--;

        //UE_LOG(LogTemp, Warning, TEXT("Amount of Cubes: %d"), TargetCubes);

        if (TargetCubes == 0 && DeadActor->ActorHasTag("Wave1"))
        {
            TargetCubes = GetTargetCubeCount(1);
            BP_SpawnNewBrickWave(0);
        }

        else if (TargetCubes == 0 && DeadActor->ActorHasTag("Wave2"))
        {
            TargetCubes = GetTargetCubeCount(2);
            BP_SpawnNewBrickWave(1);
        }

        else if (TargetCubes == 0 && DeadActor->ActorHasTag("Wave3"))
        {
            TargetCubes = GetTargetCubeCount(3);
            BP_SpawnNewBrickWave(2);
        }

        else if (TargetCubes == 0 && DeadActor->ActorHasTag("Wave4"))
        {
            OnPlayerWin.Broadcast();
        }

    }    
}

void ABomberKiddoGameMode::BeginPlay()
{
    Super::BeginPlay();

    // Inicializa la configuracion que se quiere en el juego
    HandleGameStart();

}

void ABomberKiddoGameMode::HandleGameStart()
{
    TargetCubes = GetTargetCubeCount(0);
   // UE_LOG(LogTemp, Warning, TEXT("Amount of Cubes: %d"), TargetCubes);
}

int32 ABomberKiddoGameMode::GetTargetCubeCount(int actualWave)
{
    // Calcula y retorna la cantidad de Cubos que hay actualmente en el escenario
    TArray<AActor*> Cubes;
    UGameplayStatics::GetAllActorsOfClass(this, ABK_Cube::StaticClass(), Cubes);

    switch (actualWave)
    {
    case 0:
        UGameplayStatics::GetAllActorsWithTag(this, "Wave1", Cubes);
        break;
    case 1:
        UGameplayStatics::GetAllActorsWithTag(this, "Wave2", Cubes);
        break;
    case 2:
        UGameplayStatics::GetAllActorsWithTag(this, "Wave3", Cubes);
        break;
    case 3:
        UGameplayStatics::GetAllActorsWithTag(this, "Wave4", Cubes);
        break;
    default:
        break;
    }

    return Cubes.Num();
}

