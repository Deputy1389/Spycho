#pragma once
#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SpychoOptions.generated.h"

UCLASS()
class SPYCHO_API USpychoOptions : public USaveGame
{
    GENERATED_BODY()
public:
    UPROPERTY(SaveGame) float Sensitivity=1.f;
    UPROPERTY(SaveGame) float MasterVolume=.9f;
    UPROPERTY(SaveGame) float EffectsVolume=1.f;
    UPROPERTY(SaveGame) float AmbientVolume=.5f;
    UPROPERTY(SaveGame) float Brightness=0.f;
    UPROPERTY(SaveGame) int32 Difficulty=1;
    void ClampValues();
};
