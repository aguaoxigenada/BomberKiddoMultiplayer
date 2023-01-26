#include "BomberKiddoCharacter.h"
#include "BomberKiddoGameMode.h"
#include "Actors/BK_Bomb.h"
#include "AI/NavigationSystemBase.h"
#include "Blueprint/WidgetTree.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/TextBlock.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameStates/MainGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Input/Events.h"
#include "Net/UnrealNetwork.h"


//////////////////////////////////////////////////////////////////////////
// ABomberKiddoCharacter

class UHorizontalBox;

 void ABomberKiddoCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
 {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	//DOREPLIFETIME_CONDITION(ABomberKiddoCharacter, bPlayerDead, COND_SkipOwner);
	//DOREPLIFETIME(ABomberKiddoCharacter, CurrentLife);
 }

ABomberKiddoCharacter::ABomberKiddoCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// is replicated
	bReplicates = true;
	bNetUseOwnerRelevancy = true;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Create a particle system
	ExplosionParticle = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ExplosionParticles"));
	ExplosionParticle->SetupAttachment(RootComponent);
	ExplosionParticle->bAutoActivate = false;

	// Create a SpawnBomb Point
	SpawnBombPoint = CreateDefaultSubobject<USceneComponent>(TEXT("SpawnBombPoint"));
	SpawnBombPoint->SetupAttachment(RootComponent);

	// Create BigBomb Point
	BigBombSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("BigBombSpawnPoint"));
	BigBombSpawnPoint->SetupAttachment(RootComponent);

}

void ABomberKiddoCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld())
	{
		CameraManager = World->GetFirstPlayerController()->PlayerCameraManager;
		CameraManager->ViewPitchMin = InViewPitchMin;
		CameraManager->ViewPitchMax = InViewPitchMax;
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void ABomberKiddoCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &ABomberKiddoCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ABomberKiddoCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	// Throw Bomb
	PlayerInputComponent->BindAction("ThrowBomb", IE_Pressed, this, &ABomberKiddoCharacter::StartThrowBomb);

	// Throw Big Bomb
	PlayerInputComponent->BindAction("ThrowBigBomb", IE_Pressed, this, &ABomberKiddoCharacter::ThrowBigBomb);

}


void ABomberKiddoCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ABomberKiddoCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ABomberKiddoCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ABomberKiddoCharacter::MoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}


void ABomberKiddoCharacter::SetMaxLife(int StartingLife)
{
	CurrentLife = StartingLife;
	MaxLife = StartingLife;

	//UE_LOG(LogTemp, Display, TEXT("CurrentLife is: %d, MaxLife is: %d"), CurrentLife, MaxLife);
}



void ABomberKiddoCharacter::GetDamage(int Damage)
{
	CurrentLife -= Damage;
	ExplosionParticle->ActivateSystem(true);
	BP_GetDamageEvent(CurrentLife);
	if (CurrentLife > 0)
	{
		// Se pueden realizar cosas.
	}
	else if (!bPlayerDead)
	{
		if(HasAuthority())
		{
			OnPlayerLost.Broadcast();
			bPlayerDead = true;
		}
		else if(IsLocallyControlled())
		{
			OnPlayerLost.Broadcast();
		}
		
		//bPlayerDead = true;
		PlayDeathSound();
	}
}

void ABomberKiddoCharacter::StartThrowBomb()
{
	// Launch Bomb
	GetWorldTimerManager().SetTimer(TimerHandle, this, &ABomberKiddoCharacter::ThrowBomb, BombStartTime, false);

}


void ABomberKiddoCharacter::ThrowBomb()
{
	TheWorld = CheckIfCanThrow();

	if (TheWorld == nullptr)
		return;

 	// Run Server Movement Animation
	if(HasAuthority())
	{
		// Se replica en todos los clientes.
		MultiCastPlayThrowBombAnim();
	
		// Spawn Small Bomb
		const FTransform SpawnPoint = SpawnBombPoint->GetComponentTransform();
		AActor* Bomb = TheWorld->SpawnActor<ABK_Bomb>(SmallBomb, SpawnPoint);
	}

	else
	{
		ClientPlayThrowBombAnim(true);
		ServerThrowBombAnim();
	}

	// SFX
	PlayLaunchBombSound();

	// Control Cooldown ThrowBomb
	GetWorldTimerManager().SetTimer(SmallBombCooldownTimeHandle, this, &ABomberKiddoCharacter::ResetCanThrowBomb, SmallBombShootCooldown, false);
	bCanThrowBomb = false;
}


void ABomberKiddoCharacter::ClientPlayThrowBombAnim_Implementation(bool bSmallBomb)
{
 	if(bSmallBomb)
	{
		PlayAnimMontage(ThrowBombAnim);
		//UE_LOG(LogTemp, Warning, TEXT("Client"));
	}

	// se puede hacer la segunda bomba
}

void ABomberKiddoCharacter::ServerThrowBombAnim_Implementation()
{
	PlayAnimMontage(ThrowBombAnim); 
	ThrowBomb();
	//UE_LOG(LogTemp, Warning, TEXT("Wants to Animate!"));
}

void ABomberKiddoCharacter::MultiCastPlayThrowBombAnim_Implementation()
{
	PlayAnimMontage(ThrowBombAnim);
}

void ABomberKiddoCharacter::ThrowBigBomb()
{
	TheWorld = CheckIfCanThrow();

	if (TheWorld == nullptr)
		return;

	// Spawn Big Bomb
	const FTransform SpawnPoint = BigBombSpawnPoint->GetComponentTransform();
	AActor* Bomb = TheWorld->SpawnActor<ABK_Bomb>(BigBomb, SpawnPoint);

	// SFX
	PlayLaunchBombSound();

	// Control Cooldown ThrowBomb
	GetWorldTimerManager().SetTimer(BigBombCooldownTimeHandle, this, &ABomberKiddoCharacter::ResetCanThrowBigBomb, BigBombShootCooldown, false);
	bCanThrowBigBomb = false;
}


UWorld* ABomberKiddoCharacter::CheckIfCanThrow()
{
	UWorld* World = GetWorld();
	if (!World || !bCanThrowBomb)
		return nullptr;

 	return World;
}

void ABomberKiddoCharacter::ResetCanThrowBomb()
{
	bCanThrowBomb = true;
}

void ABomberKiddoCharacter::ResetCanThrowBigBomb()
{
	bCanThrowBigBomb = true;
}

void ABomberKiddoCharacter::StopMovement()
{
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
}

void ABomberKiddoCharacter::PlayLaunchBombSound()
{
	if (LaunchBombSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, LaunchBombSound, GetActorLocation());
	}
}

void ABomberKiddoCharacter::PlayDeathSound()
{
	if (DeathSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation());
	}
}

bool ABomberKiddoCharacter::GetCanThrowBomb()
{
	// Lo puedo usar en el futuro
	return bCanThrowBomb;
}