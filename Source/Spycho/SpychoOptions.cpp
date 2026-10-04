#include "SpychoOptions.h"
void USpychoOptions::ClampValues()
{
    Sensitivity=FMath::Clamp(Sensitivity,.25f,2.5f);
    MasterVolume=FMath::Clamp(MasterVolume,0.f,1.f);
    EffectsVolume=FMath::Clamp(EffectsVolume,0.f,1.f);
    AmbientVolume=FMath::Clamp(AmbientVolume,0.f,1.f);
    Brightness=FMath::Clamp(Brightness,-.5f,1.f);
    Difficulty=FMath::Clamp(Difficulty,0,2);
}
