// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerCharacter.h"


// Sets default values
APlayerCharacter::APlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 350.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.5f;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 300.0f;
	SpringArm->bUsePawnControlRotation = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->SetRelativeLocation(FVector(0, 90, 50));
	Camera->FieldOfView = 80;
	Camera->bUsePawnControlRotation = false;

}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	Params.AddIgnoredActor(this);
}

void APlayerCharacter::MoveRight(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		const FRotator Rotation = GetWorld()->GetFirstPlayerController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
	}
}

void APlayerCharacter::MoveUp(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		const FRotator Rotation = GetWorld()->GetFirstPlayerController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}

}

void APlayerCharacter::Aim(float Value)
{
	if ((Controller != NULL) && (Value > 0.0f))
	{
		Camera->FieldOfView = 70;
	//	SetActorRotation(FMath::Lerp(FQuat(GetActorRotation()),FQuat(Camera->GetForwardVector().ToOrientationRotator()),GetWorld()->GetDeltaSeconds() * 20));
		SetActorRotation(Camera->GetForwardVector().ToOrientationRotator());
		Camera->SetRelativeLocation(FVector(0, 80, 65));
		SpringArm->TargetArmLength = 150;
	}
	else if ((Controller != NULL) && (Value == 0))
	{
		Camera->FieldOfView = 80;
		SetActorRotation(FMath::Lerp(FQuat(GetActorRotation()), FQuat(FRotator(0, GetActorRotation().Yaw, GetActorRotation().Roll)), GetWorld()->GetDeltaSeconds() * 20));
		Camera->SetRelativeLocation(FVector(0, 90, 50));
		SpringArm->TargetArmLength = 300;
	}
}

void APlayerCharacter::Sprint(float Value)
{
	if ((Controller != NULL) && (Value > 0.0f))
	{
		GetCharacterMovement()->MaxWalkSpeed = 1000;
		GetCharacterMovement()->RotationRate = FRotator(0.0f, 250.0f, 0.0f);
	}
	else if ((Controller != NULL) && (Value == 0))
	{
		GetCharacterMovement()->MaxWalkSpeed = 600;
		GetCharacterMovement()->RotationRate = FRotator(0.0f, 350.0f, 0.0f);
	}
}

void APlayerCharacter::Fire(float Value)
{
	if ((Controller != NULL) && (Value > 0.0f))
	{	
		if (RaytraceHit && Hit.GetActor() && bCanShoot)
		{
			GetWorld()->SpawnActor<AActor>(Actor, Hit.Location, FRotator(0, 0, 0), SpawnParams);
			GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Blue, FString::Printf(TEXT("Hit: %s"), *Hit.GetActor()->GetName(), true));
			bCanShoot = false;
		}
		if (RaytraceHit && Hit.GetActor() && bCanShoot)
		{
			GetWorld()->SpawnActor<AActor>(Actor, Hit.Location, FRotator(0, 0, 0), SpawnParams);
			GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Blue, FString::Printf(TEXT("Hit: %s"), *Hit.GetActor()->GetName(), true));
			bCanShoot = false;
		}
	}
}

void APlayerCharacter::Reload()
{
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (fShootAlpha < 1.f) { fShootAlpha += DeltaTime / 0.1f; }
	if (fShootAlpha >= 1.f)
	{
		fShootAlpha -= 1.f;
		bCanShoot = true;
	}
	if (bCanShoot && GetInputAxisValue("Aim") > 0)
	{
		RaytraceHit = GetWorld()->LineTraceSingleByChannel(Hit, Camera->GetComponentLocation(), (Camera->GetComponentRotation().Vector() * 5000.f) + Camera->GetComponentLocation(), ECC_Visibility, Params);
		//DrawDebugLine(GetWorld(), Camera->GetComponentLocation(), (Camera->GetComponentRotation().Vector() * 5000.f) + Camera->GetComponentLocation(), FColor::Red, false, 0, 0, 2);
	}
	else if (bCanShoot && GetInputAxisValue("Aim") <= 0)
	{
		RaytraceHit = GetWorld()->LineTraceSingleByChannel(Hit, GetActorLocation(), (GetActorForwardVector() * 5000.f) + GetActorLocation(), ECC_Visibility, Params);
		//DrawDebugLine(GetWorld(), GetActorLocation(), (GetActorForwardVector() * 5000.f) + GetActorLocation(), FColor::Red, false, 0, 0, 2);
	}
	
}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	InputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	InputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	InputComponent->BindAxis("Up", this, &APlayerCharacter::MoveUp);
	InputComponent->BindAxis("Right", this, &APlayerCharacter::MoveRight);

	InputComponent->BindAxis("Aim", this, &APlayerCharacter::Aim);

	InputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	InputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	InputComponent->BindAxis("Sprint", this, &APlayerCharacter::Sprint);

	InputComponent->BindAxis("Fire", this, &APlayerCharacter::Fire);
	//InputComponent->BindAction("Reload", this, &APlayerCharacter::Reload);

}

