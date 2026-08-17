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

	InputForceScale = FVector(8000.0f, 8000.0f, 15000.0f);
	Mass = 10.0f;

	Velocity = FVector::ZeroVector;
	Acceleration = FVector::ZeroVector;
	InputForce = FVector::ZeroVector;
	InAirControlScale = 0.5f;
	bIsInAir = true;
}

void AMyPawn::BeginPlay()
{
	Super::BeginPlay();
}

void AMyPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 1. 힘 합산
	const FVector TotalForce =
		InputForce +                                                                           // 입력 힘
		(bIsInAir ? FVector(0.0f, 0.0f, -GravityAcceleration * Mass) : FVector::ZeroVector) +  // 중력
		-Velocity * Velocity.Size() * DragCoefficient;                                         // 공기 저항

	// 2. 가속도 업데이트
	Acceleration = TotalForce / Mass;

	// 3. 속도 업데이트
	Velocity += Acceleration * DeltaTime;

	// 4. 이동 확인
	const FVector ControlScale = bIsInAir
									 ? FVector(InAirControlScale, InAirControlScale, 1.0f)
									 : FVector(1.0f, 1.0f, 1.0f);

	const FVector DeltaLocation = Velocity * ControlScale * DeltaTime;
	if (DeltaLocation.IsNearlyZero())
	{
		InputForce = FVector::ZeroVector;
		return;
	}

	// 5. 충돌 검사
	const FVector StartLocation = GetActorLocation();

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	const float CapsuleRadius = CapsuleComp->GetScaledCapsuleRadius();
	const float CapsuleHalfHeight = CapsuleComp->GetScaledCapsuleHalfHeight();
	const FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight);

	// 6. 위치 업데이트
	FVector CurrentDelta = DeltaLocation;

	// 최대 3회 루프 (코너나 경사면 등 확인)
	bIsInAir = true;
	for (int32 Iteration = 0; Iteration < 3; ++Iteration)
	{
		// 매 루프마다 현재 위치 기준의 최종 목적지 계산
		const FVector EndLocation = GetActorLocation() + CurrentDelta;

		// SweepSingleByChannel을 사용해 충돌 테스트
		const bool bLoopHit = GetWorld()->SweepSingleByChannel(
			HitResult,
			GetActorLocation(),
			EndLocation,
			FQuat::Identity,
			ECollisionChannel::ECC_Visibility,
			CapsuleShape,
			QueryParams);

		if (bLoopHit)  // 충돌 발생
		{
			// 겹친 경우 위치 보정
			if (HitResult.bStartPenetrating)
			{
				FVector DepenetrationVector = HitResult.Normal * (HitResult.PenetrationDepth + 0.01f);
				AddActorWorldOffset(DepenetrationVector, false);
				bIsInAir = false;
				continue;  // 위치가 수정되었으므로 다음 루프에서 다시 충돌 테스트 시도
			}

			// 충돌 직전까지 실제로 이동
			FVector MoveToWall = CurrentDelta * HitResult.Time;
			AddActorWorldOffset(MoveToWall, false);

			// 충돌 직전까지 남은 이동 거리 계산
			FVector RemainingDelta = CurrentDelta * (1.0f - HitResult.Time);

			// 부딪힌 표면이 걸어갈 수 있는 바닥(경사면)인지 판단 (Normal.Z가 0.7 이상이면 약 45도 이하 경사)
			const bool bIsWalkableFloor = HitResult.Normal.Z > 0.7f;
			if (bIsWalkableFloor)  // 바닥 O - 수평 이동 속도를 경사면에 평행하게 투영 (속도 저하 해결)
			{
				CurrentDelta = FVector::VectorPlaneProject(RemainingDelta, HitResult.Normal);
			}
			else  // 바닥 X
			{
				float PlaneDot = FVector::DotProduct(RemainingDelta, HitResult.Normal);
				if (PlaneDot < 0.0f)
				{
					CurrentDelta = RemainingDelta - (HitResult.Normal * PlaneDot);
				}
				else
				{
					break;
				}
			}

			// 속도(Velocity) 변수도 표면 법선에 맞춰 평면 투영으로 동기화 (감속 방지)
			Velocity = FVector::VectorPlaneProject(Velocity, HitResult.Normal);
		}
		else
		{
			// 더 이상 충돌이 없으므로 남은 이동량만큼 최종 이동 후 루프 종료
			AddActorWorldOffset(CurrentDelta, false);
			break;
		}
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
