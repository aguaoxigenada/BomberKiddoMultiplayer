#include "BK_Cube.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/BoxComponent.h"
#include "../BomberKiddoGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
ABK_Cube::ABK_Cube()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//init Mesh
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;

	//Init BoxComponent
	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	BoxCollision->SetupAttachment(Mesh);
	BoxCollision->SetCollisionProfileName(TEXT("OverlapAll"));

	// Create a particle system
	ExplosionParticle = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ExplosionParticles"));
	ExplosionParticle->SetupAttachment(RootComponent);

	CurrentLife = MaxLife;
}

void ABK_Cube::BeginPlay()
{
	Super::BeginPlay();

	StartLocation = GetActorLocation();

	ExplosionParticle->SetHiddenInGame(true);

	BomberKiddoGameMode = Cast<ABomberKiddoGameMode>(UGameplayStatics::GetGameMode(this));
}

void ABK_Cube::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	MovePlatform(DeltaTime);
	RotatePlatform(DeltaTime);
}

void ABK_Cube::MovePlatform(float DeltaTime)
{
	// Necesita un chequeo de puntero del personaje, porque al morir el puntero desaparece.
	if (ShouldPlatformReturn())
	{
		FVector MoveDirection = PlatformVelocity.GetSafeNormal();
		StartLocation = StartLocation + MoveDirection * MoveDistance;
		SetActorLocation(StartLocation);
		PlatformVelocity = -PlatformVelocity;
	}
	else
	{
		// Mover el Cubo hacia adelante
		FVector CurrentLocation = GetActorLocation();
		CurrentLocation = CurrentLocation + (PlatformVelocity * DeltaTime);
		SetActorLocation(CurrentLocation);
	}
}

void ABK_Cube::RotatePlatform(float DeltaTime)
{
	AddActorLocalRotation(RotationVelocity * DeltaTime);
}

bool ABK_Cube::ShouldPlatformReturn() const
{
	return GetDistanceMoved() > MoveDistance;
}

float ABK_Cube::GetDistanceMoved() const
{
	return FVector::Dist(StartLocation, GetActorLocation());
}

void ABK_Cube::GetDamage(int Damage)
{
	CurrentLife -= Damage;
	ExplosionParticle->SetHiddenInGame(false);
	TimerDelegate.BindLambda([&]
		{
		 // Desaparece el Fueguito encima del cubo en 3 sec
			ExplosionParticle->SetHiddenInGame(true);
		});

	if (CurrentLife > 0)
	{
		// Podria cambiar de animacion o cualquier otra cosa del cubo.
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, 3, false);
	}
	else
	{
		BomberKiddoGameMode->ActorDied(this);
		Destroy();
	}
}

void ABK_Cube::HandleDestruction()
{
	if (DeathParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(this, DeathParticles, GetActorLocation(), GetActorRotation());
	}

	if (DeathSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation());
	}
}