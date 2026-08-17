#include "MyPawn.h"

#include "EnhancedInputComponent.h"
#include "MyPlayerController.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"

AMyPawn::AMyPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	SetRootComponent(CapsuleComp);
	CapsuleComp->SetSimulatePhysics(false);  // 물리 대신 코드로 직접 제어

	SkeletalMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Skeletal Mesh"));
	SkeletalMeshComp->SetupAttachment(CapsuleComp);
	SkeletalMeshComp->SetSimulatePhysics(false);  // 물리 대신 코드로 직접 제어

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	SpringArmComp->SetupAttachment(CapsuleComp);

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComp->SetupAttachment(SpringArmComp);

	MoveOffset = FVector(30.0f, 0.0f, 0.0f);
}

void AMyPawn::BeginPlay()
{
	Super::BeginPlay();
}

void AMyPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AddActorWorldOffset(MoveOffset * DeltaTime, true);
}

void AMyPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (AMyPlayerController* PlayerController = Cast<AMyPlayerController>(GetController()))
		{
			if (PlayerController->MoveInputAction)
			{
				EnhancedInput->BindAction(
					PlayerController->MoveInputAction,
					ETriggerEvent::Triggered,
					this,
					&AMyPawn::Move);
			}

			if (PlayerController->LookInputAction)
			{
				EnhancedInput->BindAction(
					PlayerController->LookInputAction,
					ETriggerEvent::Triggered,
					this,
					&AMyPawn::Look);
			}
		}
	}
}

void AMyPawn::Move(const FInputActionValue& value)
{
	if (!Controller) return;

	const FVector& MoveInput = value.Get<FVector>();

	if (!FMath::IsNearlyZero(MoveInput.X))
	{
		AddActorLocalOffset(GetActorForwardVector() * MoveInput.X, true);
	}

	if (!FMath::IsNearlyZero(MoveInput.Y))
	{
		AddActorLocalOffset(GetActorRightVector() * MoveInput.Y, true);
	}

	if (!FMath::IsNearlyZero(MoveInput.Z))
	{
		AddActorLocalOffset(GetActorUpVector() * MoveInput.Z, true);
	}
}

void AMyPawn::Look(const FInputActionValue& value)
{
	if (!Controller) return;

	const FVector& LookInput = value.Get<FVector>();

	FRotator DeltaRotator(LookInput.Y, LookInput.X, LookInput.Z);
	AddActorLocalRotation(DeltaRotator, true);
}
