#include "BK_Bomb.h"
#include "BK_Cube.h"
#include "../BomberKiddoCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"

#pragma optimize("", off)
// Sets default values
ABK_Bomb::ABK_Bomb() // constructor
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Init Mesh
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;

	// Init SphereComponent
	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereCollision->SetupAttachment(Mesh);
	SphereCollision->InitSphereRadius(0.f);

	// Init Bomb Spark Particles
	BombSparkParticle = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("SparkParticles"));
	BombSparkParticle->SetupAttachment(Mesh);

	// Init Explosion Particles
	ExplosionParticle = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ExplosionParticles"));
	ExplosionParticle->SetupAttachment(Mesh);  
	ExplosionParticle->bAutoActivate = false;

	// is replicated
	bReplicates = true;
	bNetUseOwnerRelevancy = true;

}

#pragma optimize("", on)
// Called when the game starts or when spawned
void ABK_Bomb::BeginPlay()
{
	Super::BeginPlay(); 

	const FVector Impulse = Mesh->GetForwardVector() * InitialForce;
	Mesh->AddImpulse(Impulse);

	if (ExplodeTime < 0.0f)
		return;

	GetWorldTimerManager().SetTimer(ExplodeTimeHandle, this, &ABK_Bomb::OnExplode, ExplodeTime, false);
}

void ABK_Bomb::OnExplode()
{
	SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &ABK_Bomb::OnComponentBeginOverlapInBomb);
	GetWorldTimerManager().SetTimer(ExplosionTimeHandle, this, &ABK_Bomb::OnFinishedExplosion, ExplosionTime, false);
	ExplosionParticle->Activate(true); 
	BombSparkParticle->SetHiddenInGame(true);
	SphereCollision->SetSphereRadius(Radius);
	Mesh->SetSimulatePhysics(false);
	Mesh->SetHiddenInGame(true);

	if (DeathSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation());
	}
}

void ABK_Bomb::OnFinishedExplosion()
{
	this->Destroy();
}


void ABK_Bomb::OnParticleSystemFinished(UParticleSystemComponent* PSystem)
{
	this->Destroy();
}

void ABK_Bomb::OnComponentBeginOverlapInBomb(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

	ABomberKiddoCharacter* Character = Cast<ABomberKiddoCharacter>(OtherActor);
	ABK_Cube* Cube = Cast<ABK_Cube>(OtherActor);
	
	if (Character && Character != PlayerDamaged)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Being Hit!"));
		BP_OnPlayerDetected(Character);
		PlayerDamaged = Character;
	}
	else if (Cube && Cube != CubeDamaged)
	{
		// Esto se podria hacer para el Character si no se
		// Quisiera usar el evento de BP_OnPlayerDetected
		Cube->GetDamage(DamageOfBombType);
		CubeDamaged = Cube;
	}
	  
	else
		return;
}

