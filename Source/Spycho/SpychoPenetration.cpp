#include "SpychoPenetration.h"
#include "SpychoSurface.h"
#include "SpychoHealth.h"
#include "SpychoRules.h"
#include "SpychoGameState.h"
#include "Components/PrimitiveComponent.h"

void USpychoPenetration::Fire(FVector Origin, FVector Direction)
{
    if (!GetOwner()->HasAuthority()) return;
    Direction.Normalize();
    ASpychoGameState* GS = GetWorld()->GetGameState<ASpychoGameState>();
    FVector Start = Origin;
    const FVector End = Origin + Direction * Range;
    float Energy = InitialEnergy;
    FCollisionQueryParams Query(SCENE_QUERY_STAT(SpychoBullet), false, GetOwner());
    Query.bReturnPhysicalMaterial = true;
    // Do not ignore whole wall actors: the house contains many distinct surfaces.
    for (int32 Event=0; Event<=MaxPenetrations && Energy>0.f; ++Event)
    {
        FHitResult Hit;
        if (!GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Query)) { if (GS) GS->ShotDebug(Start, End, TEXT("miss")); break; }
        if (auto* Health = Hit.GetActor() ? Hit.GetActor()->FindComponentByClass<USpychoHealth>() : nullptr)
        {
            float Damage = SpychoRules::Damage(Energy, InitialEnergy, BaseDamage); Health->Damage(Damage,GetOwner(),Event);
            if (GS) GS->ShotDebug(Start, Hit.ImpactPoint, FString::Printf(TEXT("target: energy %.1f, damage %.1f"), Energy, Damage));
            break;
        }
        if (GS) { GS->Impact(Hit.ImpactPoint, Hit.ImpactNormal, false); GS->Noise(ESpychoNoise::Impact, Hit.ImpactPoint, 0.3f); }
        const USpychoSurface* Surface = Cast<USpychoSurface>(Hit.PhysMaterial.Get());
        if (!Surface || !Surface->bPenetrable || Event == MaxPenetrations)
        {
            if (GS) GS->ShotDebug(Start, Hit.ImpactPoint, TEXT("stopped: structural or penetration limit")); break;
        }
        // Reverse trace the exact component from beyond its bounds. Convex graybox
        // surfaces give the real exit along the ray, including oblique shots.
        UPrimitiveComponent* Component = Hit.GetComponent();
        if (!Component) break;
        const float Reach = Component->Bounds.BoxExtent.Size() * 2.f + 10.f;
        FHitResult Exit;
        FCollisionQueryParams Reverse(SCENE_QUERY_STAT(SpychoExit), false);
        if (!Component->LineTraceComponent(Exit, Hit.ImpactPoint + Direction * Reach, Hit.ImpactPoint - Direction, Reverse)) break;
        const float Thickness = FVector::DotProduct(Exit.ImpactPoint - Hit.ImpactPoint, Direction);
        if (Thickness <= 0.1f || Thickness > Surface->MaxThickness) { if (GS) GS->ShotDebug(Start, Hit.ImpactPoint, TEXT("stopped: too thick")); break; }
        const float Before = Energy;
        Energy = SpychoRules::RemainingEnergy(Energy, Thickness, Surface->ResistancePerCm, Surface->EntryCost);
        if (GS) GS->ShotDebug(Start, Exit.ImpactPoint, FString::Printf(TEXT("%s: %.1f cm, energy %.1f -> %.1f"), *Surface->GetName(), Thickness, Before, Energy));
        if (Energy<=0.f) break;
        if (GS) GS->Impact(Exit.ImpactPoint, Exit.ImpactNormal, true);
        Start = Exit.ImpactPoint + Direction * 0.5f;
        if (FVector::DotProduct(End-Start, Direction)<=0.f) break;
    }
}
