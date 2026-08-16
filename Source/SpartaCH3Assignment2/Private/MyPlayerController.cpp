#include "MyPlayerController.h"

#include "EnhancedInputSubsystems.h"

AMyPlayerController::AMyPlayerController()
	: DefaultInputMappingContext(nullptr),
	  MoveInputAction(nullptr),
	  LookInputAction(nullptr)
{
}

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (DefaultInputMappingContext)
			{
				Subsystem->AddMappingContext(DefaultInputMappingContext, 0);
			}
		}
	}
}
