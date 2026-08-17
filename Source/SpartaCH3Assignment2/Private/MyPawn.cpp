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

	GravityAcceleration = 980.0f;
	Velocity = FVector::ZeroVector;
}

void AMyPawn::BeginPlay()
{
	Super::BeginPlay();
}

void AMyPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// AddActorWorldOffset(MoveOffset * DeltaTime, true);

	// 충돌 감지
	FHitResult HitResult;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	const FVector& StartLocation = GetActorLocation();
	const FVector& EndLocation = StartLocation + FVector(0.0f, 0.0f, -1.0f) * (CapsuleComp->GetScaledCapsuleHalfHeight() + 5.0f);

	bool bSuccess = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		StartLocation,
		EndLocation,
		ECollisionChannel::ECC_Visibility,
		QueryParams);
	if (bSuccess)  // 지면 충돌 O -> Z축 속도 0
	{
		Velocity.Z = 0.0f;
	}
	else  // 지면 충돌 X -> 중력 적용
	{
		Velocity.Z += -GravityAcceleration * DeltaTime;            // v = v0 + a * t
		Velocity.Z = FMath::Clamp(Velocity.Z, -4000.0f, 4000.0f);  // 종단속도 설정
		const FVector& DeltaLocation = Velocity * DeltaTime;       // s = v * t
		AddActorWorldOffset(DeltaLocation, true);                  // 월드 기준 아래로 이동
	}
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
