
#pragma once

#include "CoreMinimal.h"

#include "GameFramework/PlayerController.h"

#include "MyPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;

UCLASS()
class SPARTACH3ASSIGNMENT2_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()

  public:
	AMyPlayerController();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inputs")
	UInputMappingContext* DefaultInputMappingContext;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inputs")
	UInputAction* MoveInputAction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inputs")
	UInputAction* LookInputAction;

  protected:
	virtual void BeginPlay() override;
};
