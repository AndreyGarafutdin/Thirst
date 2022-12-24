#include "ThirstToLiveMainCharacter.h"
#include "DrawDebugHelpers.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"


// Sets default values
AThirstToLiveMainCharacter::AThirstToLiveMainCharacter():
BaseTurnRate(45.f),
BaseLookUpRate(45.f),
bAiming(false),
CameraDefaultFOV(0.f), // set in BeginPlay
CameraZoomedFOV(40.f),
CameraCurrentFOV(0.f),
ZoomInterpSpeed(20.f)
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Create camera boom (pulls in towards the character if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom -> SetupAttachment(RootComponent);
	CameraBoom -> TargetArmLength = 200.f; //camera distance
	CameraBoom -> bUsePawnControlRotation = true; // Rotate the arm based on the controller
	CameraBoom ->SocketOffset = FVector(0.f, 50.f, 70.f);

	//Create Follow Camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera -> SetupAttachment(CameraBoom, USpringArmComponent::SocketName); //attach camera to end of CameraBoom
	FollowCamera -> bUsePawnControlRotation = false;

	//Don't rotate mesh when controller rotates. Let the controller only affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = true;

	//Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false; //Character moves in direction of input...
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f); //... at this rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

}

// Called when the game starts or when spawned
// Called when the game starts or when spawned
void AThirstToLiveMainCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (FollowCamera)
	{
		CameraDefaultFOV = GetFollowCamera()->FieldOfView;
		CameraCurrentFOV = CameraDefaultFOV;
	}
}

void AThirstToLiveMainCharacter::MoveForward(float Value)
{
	if (Controller != nullptr && (Value != 0.0f))
	{
		//find out which way is forward
		const FRotator Rotation{Controller->GetControlRotation()};
		FRotator YawRotation {0, Rotation.Yaw, 0};

		const FVector Direction {FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::X)};
		AddMovementInput(Direction,Value);
	}
}

void AThirstToLiveMainCharacter::MoveRight(float Value)
{
	if (Controller != nullptr && (Value != 0.0f))
	{
		//find out which way is right
		const FRotator Rotation{Controller->GetControlRotation()};
		FRotator YawRotation {0, Rotation.Yaw, 0};

		const FVector Direction {FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::Y)};
		AddMovementInput(Direction,Value);
	}
}

void AThirstToLiveMainCharacter::TurnRate(float Rate)
{
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AThirstToLiveMainCharacter::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AThirstToLiveMainCharacter::FireWeapon()
{
	if (FireSound)
	{
		UE_LOG(LogTemp, Warning, TEXT("Fire"));
		UGameplayStatics::PlaySound2D(this,FireSound);
	}
	const USkeletalMeshSocket* BarrelSocket = GetMesh()->GetSocketByName("BarrelSocket");
	if (BarrelSocket)
	{
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(GetMesh());
		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);
		}

		FVector BeamEnd;
		bool bBeamEnd = GetBeamEndLocation(SocketTransform.GetLocation(), BeamEnd);
		if (bBeamEnd)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),ImpactParticles,BeamEnd);
		}

		UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, SocketTransform);
		if (Beam)
		{
			Beam->SetVectorParameter(FName ("Target"),BeamEnd);
		}
	}
	
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HipFireMontage)
	{
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}
}

bool AThirstToLiveMainCharacter::GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation)
{
	//Get current size of viewport
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}
	
	//Get screen space location of crosshairs
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	CrosshairLocation.Y -= 50.f;
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	
	//GetWorld position and direction of crosshairs
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection);

	if (bScreenToWorld) // Was deprojection successful?
		{
		FHitResult ScreenTraceHit;
		const FVector Start {CrosshairWorldPosition};
		const FVector End {CrosshairWorldPosition + CrosshairWorldDirection * 50'000.f};

		//Set beam end point to line trace end point
		OutBeamLocation = End;

		//Trace outward from crosshairs world location
		GetWorld()->LineTraceSingleByChannel(ScreenTraceHit,
			Start,
			End,
			ECollisionChannel::ECC_Visibility);
		if (ScreenTraceHit.bBlockingHit) //was there a trace hit?
			{
			OutBeamLocation = ScreenTraceHit.Location;		
			}

		//Perform a second trace, this time from gun barrel
		FHitResult WeaponTraceHit;
		const FVector WeaponTraceStart {MuzzleSocketLocation};
		const FVector WeaponTraceEnd {OutBeamLocation};
		GetWorld()->LineTraceSingleByChannel(
			WeaponTraceHit,
			WeaponTraceStart,
			WeaponTraceEnd,
			ECollisionChannel::ECC_Visibility);
			
		if (WeaponTraceHit.bBlockingHit) // Object between barrel and BeamEndPoint?
			{
			OutBeamLocation = WeaponTraceHit.Location;
			}
		return true;
		}
	return false;
}

void AThirstToLiveMainCharacter::AimingButtonPressed()
{
	bAiming = true;
}

void AThirstToLiveMainCharacter::AimingButtonReleased()
{
	bAiming = false;
}

void AThirstToLiveMainCharacter::CameraInterpolationZoom(float DeltaTime)
{	
	//Set camera Field Of View
	if (bAiming)
	{
		//Interpolation to zoomed camera FOV
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV,CameraZoomedFOV,DeltaTime,ZoomInterpSpeed);		
	}
	else
	{
		//Interpolation to default camera FOV
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV,CameraDefaultFOV,DeltaTime,ZoomInterpSpeed);		
	}
	GetFollowCamera()->SetFieldOfView(CameraCurrentFOV);
}

// Called every frame
void AThirstToLiveMainCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Handle interpolation for zoom when aiming
	CameraInterpolationZoom(DeltaTime);

}

// Called to bind functionality to input
void AThirstToLiveMainCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AThirstToLiveMainCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AThirstToLiveMainCharacter::MoveRight);
	PlayerInputComponent->BindAxis("TurnRate", this, &AThirstToLiveMainCharacter::TurnRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AThirstToLiveMainCharacter::LookUpAtRate);
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("FireButton", IE_Pressed, this, &AThirstToLiveMainCharacter::FireWeapon);

	PlayerInputComponent->BindAction("AimingButton", IE_Pressed, this, &AThirstToLiveMainCharacter::AimingButtonPressed);
	PlayerInputComponent->BindAction("AimingButton", IE_Released, this, &AThirstToLiveMainCharacter::AimingButtonReleased);
	PlayerInputComponent->BindAction("AimingButton", IE_Released, this, &AThirstToLiveMainCharacter::AimingButtonReleased);
}
