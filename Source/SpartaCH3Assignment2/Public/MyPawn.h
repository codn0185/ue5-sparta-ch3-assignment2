#pragma once

#include "CoreMinimal.h"

#include "GameFramework/Pawn.h"

#include "MyPawn.generated.h"

class UCapsuleComponent;
class USpringArmComponent;
class UCameraComponent;

struct FInputActionValue;

UCLASS()
class SPARTACH3ASSIGNMENT2_API AMyPawn : public APawn
{
	GENERATED_BODY()

  public:
	AMyPawn();

  protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UCapsuleComponent* CapsuleComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	USkeletalMeshComponent* SkeletalMeshComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	USpringArmComponent* SpringArmComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UCameraComponent* CameraComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
	float GravityAcceleration;  // 중력가속도 (cm/s^2)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
	float DragCoefficient;  // 공기저항 계수

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
	FVector InputForceScale;  // 입력 힘 배율
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
	float Mass;  // 질량 (kg)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
	FVector Velocity;  // 속도 (cm/s)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
	FVector Acceleration;  // 가속도 (cm/s^2)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
	FVector InputForce;  // 입력 힘 (F = m * a)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Properties")
	bool bIsFalling;  // 공중 여부

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION()
	virtual void Move(const FInputActionValue& value);
	UFUNCTION()
	virtual void Look(const FInputActionValue& value);
};
