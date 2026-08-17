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
	CapsuleComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	SkeletalMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Skeletal Mesh"));
	SkeletalMeshComp->SetupAttachment(CapsuleComp);
	SkeletalMeshComp->SetSimulatePhysics(false);  // 물리 대신 코드로 직접 제어

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	SpringArmComp->SetupAttachment(CapsuleComp);

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComp->SetupAttachment(SpringArmComp);

	GravityAcceleration = 980.0f;
	DragCoefficient = 0.02f;

	InputForceScale = FVector(5000.0f, 5000.0f, 15000.0f);
	Mass = 10.0f;

	Velocity = FVector::ZeroVector;
	Acceleration = FVector::ZeroVector;
	InputForce = FVector::ZeroVector;
	bIsFalling = true;
}

void AMyPawn::BeginPlay()
{
	Super::BeginPlay();
}

void AMyPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 1. 힘 합산
	FVector TotalForce =
		InputForce +                                                                             // 입력 힘
		(bIsFalling ? FVector(0.0f, 0.0f, -GravityAcceleration * Mass) : FVector::ZeroVector) +  // 중력
		-Velocity * Velocity.Size() * DragCoefficient;                                           // 공기 저항

	// 2. 가속도 업데이트
	const FVector TargetAcceleration = TotalForce / Mass;
	Acceleration = TargetAcceleration;
	/*float DynamicSpeed = 1000.0f * 2.0f * Acceleration.Size();

	Acceleration = FMath::VInterpConstantTo(
		Acceleration,
		TargetAcceleration,
		DeltaTime,
		DynamicSpeed);*/

	// 3. 속도 업데이트
	Velocity += Acceleration * DeltaTime;

	// 4. 이동 확인
	FVector DeltaLocation = Velocity * DeltaTime;
	if (DeltaLocation.IsNearlyZero())
	{
		InputForce = FVector::ZeroVector;
		return;
	}

	// 5. 충돌 검사
	const FVector StartLocation = GetActorLocation();
	const FVector EndLocation = StartLocation + DeltaLocation;

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	const float CapsuleRadius = CapsuleComp->GetScaledCapsuleRadius();
	const float CapsuleHalfHeight = CapsuleComp->GetScaledCapsuleHalfHeight();
	const FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight);

	const bool bHit = GetWorld()->SweepSingleByChannel(
		HitResult,
		StartLocation,
		EndLocation,
		FQuat::Identity,
		ECollisionChannel::ECC_Visibility,
		CapsuleShape,
		QueryParams);

	bIsFalling = true;

	// 6. 위치 업데이트
	if (bHit)  // 지면 충돌 - 충돌 직전까지 이동 및 속도/가속도 초기화
	{
		const float SafeDistance = FMath::Max(HitResult.Distance * 0.95f, 0.0f);
		const FVector SafeMove = DeltaLocation.GetSafeNormal() * SafeDistance;
		AddActorWorldOffset(SafeMove, false);

		if (HitResult.ImpactNormal.Z > 0.7f)
		{
			Velocity.Z = FMath::Max(Velocity.Z, 0.0f);
			Acceleration.Z = FMath::Max(Acceleration.Z, 0.0f);
			bIsFalling = false;
		}
	}
	else
	{
		AddActorWorldOffset(DeltaLocation, false);
	}

	// 7. 입력 힘 초기화
	InputForce = FVector::ZeroVector;
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

	FVector DeltaLocalForce;
	if (!FMath::IsNearlyZero(MoveInput.X))
	{
		DeltaLocalForce.X = InputForceScale.X * MoveInput.X;
	}
	if (!FMath::IsNearlyZero(MoveInput.Y))
	{
		DeltaLocalForce.Y = InputForceScale.Y * MoveInput.Y;
	}
	if (!FMath::IsNearlyZero(MoveInput.Z))
	{
		DeltaLocalForce.Z = InputForceScale.Z * MoveInput.Z;
	}

	InputForce = GetActorTransform().TransformVectorNoScale(DeltaLocalForce);

	UE_LOG(LogTemp, Warning, TEXT("MoveInput: %s"), *MoveInput.ToString());
}

void AMyPawn::Look(const FInputActionValue& value)
{
	if (!Controller) return;

	const FVector& LookInput = value.Get<FVector>();

	FRotator DeltaRotator(LookInput.Y, LookInput.X, LookInput.Z);
	AddActorLocalRotation(DeltaRotator, true);
}
