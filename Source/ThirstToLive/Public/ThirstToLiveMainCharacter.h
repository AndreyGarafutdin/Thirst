// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ThirstToLiveMainCharacter.generated.h"

UCLASS()
class THIRSTTOLIVE_API AThirstToLiveMainCharacter : public ACharacter
{
		GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AThirstToLiveMainCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//Move inputs
	void MoveForward(float Value);
	void MoveRight(float Value);

	//Look around
	void TurnRate(float Rate);
	void LookUpAtRate(float Rate);

	//Called when Fire Button is pressed
	void FireWeapon();

	bool GetBeamEndLocation (const FVector& MuzzleSocketLocation, FVector& OutBeamLocation);

	// Set bAiming to true or false with button press
	void AimingButtonPressed();
	void AimingButtonReleased();

	void CameraInterpolationZoom( float DeltaTime);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	//Camera boom positioning the camera behind the character
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera,meta = (AllowPrivateAccess = "true"));
	float BaseTurnRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera, meta = (AllowPrivateAccess = "true"));
	float BaseLookUpRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Combat, meta = (AllowPrivateAccess = "true"))
	class USoundCue* FireSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Combat, meta = (AllowPrivateAccess = "true"))
	class UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Combat, meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* HipFireMontage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Combat, meta = (AllowPrivateAccess = "true"))
	UParticleSystem* ImpactParticles;

	//Smoke trail for bullets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Combat, meta = (AllowPrivateAccess = "true"))
	UParticleSystem* BeamParticles;

	//True when aiming
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Combat, meta = (AllowPrivateAccess = "true"))
	bool bAiming;

	// Default camera Field Of View value
	float CameraDefaultFOV;

	// Field Of View value for when zoomed in
	float CameraZoomedFOV;

	//Current Field Of View this frame
	float CameraCurrentFOV;

	//Interpolation speed for zooming when aiming
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Combat, meta = (AllowPrivateAccess = "true"))
	float ZoomInterpSpeed;
	
public:
	//Returns CameraBoom subobject
	FORCEINLINE USpringArmComponent* GetCameraBoom() const {return CameraBoom;}

	//Returns FollowCamera subobject
	FORCEINLINE UCameraComponent* GetFollowCamera() const {return FollowCamera;}
	
};