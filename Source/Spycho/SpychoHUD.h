#pragma once
#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SpychoHUD.generated.h"
UCLASS()
class SPYCHO_API ASpychoHUD : public AHUD
{
    GENERATED_BODY()
public:
    virtual void DrawHUD() override;
};
