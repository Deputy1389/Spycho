#pragma once
#include "CoreMinimal.h"

// Pure rules shared by gameplay and automation. Units: cm, energy, damage.
namespace SpychoRules
{
    inline float RemainingEnergy(float Energy, float Thickness, float Resistance, float EntryCost)
    {
        return FMath::Max(0.f, Energy - FMath::Max(0.f, Thickness) * FMath::Max(0.f, Resistance) - FMath::Max(0.f, EntryCost));
    }
    inline float Damage(float Energy, float Initial, float BaseDamage)
    {
        return Initial > 0.f ? BaseDamage * FMath::Clamp(Energy / Initial, 0.f, 1.f) : 0.f;
    }
    inline int32 Reload(int32& Magazine, int32& Reserve, int32 Capacity)
    {
        int32 Count = FMath::Clamp(Capacity - Magazine, 0, FMath::Max(0, Reserve));
        Magazine += Count; Reserve -= Count; return Count;
    }
    inline bool CanFire(int32 Magazine, bool Reloading, bool Alive, bool RoundActive)
    {
        return Magazine > 0 && !Reloading && Alive && RoundActive;
    }
}
