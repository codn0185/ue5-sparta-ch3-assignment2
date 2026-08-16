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

	SkeletalMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Skeletal Mesh"));
	SkeletalMeshComp->SetupAttachment(CapsuleComp);
	SkeletalMeshComp->SetSimulatePhysics(false);  // 물리 대신 코드로 직접 제어

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	SpringArmComp->SetupAttachment(CapsuleComp);

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComp->SetupAttachment(SpringArmComp);
}

void AMyPawn::BeginPlay()
{
	Super::BeginPlay();
}

void AMyPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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
}

void AMyPawn::Look(const FInputActionValue& value)
{
	if (!Controller) return;
}
