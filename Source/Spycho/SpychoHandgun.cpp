#include "SpychoHandgun.h"
#include "SpychoCharacter.h"
#include "SpychoHealth.h"
#include "SpychoRules.h"
#include "SpychoPenetration.h"
#include "SpychoGameState.h"
#include "Camera/CameraComponent.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

USpychoHandgun::USpychoHandgun() { SetIsReplicatedByDefault(true); }
void USpychoHandgun::Fire(FRotator Aim)
{
    auto* C = Cast<ASpychoCharacter>(GetOwner());
    auto* GS = GetWorld()->GetGameState<ASpychoGameState>();
    if (!C || !C->HasAuthority() || Aim.ContainsNaN() || !SpychoRules::CanFire(Magazine, bReloading, C->Health->Health>0.f, GS && GS->bRoundActive) || GetWorld()->GetTimeSeconds()<NextShot) return;
    NextShot = GetWorld()->GetTimeSeconds() + ShotInterval; --Magazine;
    // Origin is always server-owned. Clients supply only a bounded unit aim rotation.
    Aim.Pitch = FMath::Clamp(FRotator::NormalizeAxis(Aim.Pitch), -85.f, 85.f);
    Aim.Roll = 0.f;
    C->Penetration->Fire(C->Camera->GetComponentLocation(), Aim.Vector());
    GS->Noise(ESpychoNoise::Gunshot, C->GetActorLocation()+FVector(0,0,60), 2.8f);
    Fired();
}
void USpychoHandgun::Fired_Implementation()
{
    if (auto* C = Cast<ASpychoCharacter>(GetOwner())) C->ShotFeedback();
}
void USpychoHandgun::Reload()
{
    auto* C = Cast<ASpychoCharacter>(GetOwner()); auto* GS = GetWorld()->GetGameState<ASpychoGameState>();
    if (!C || !C->HasAuthority() || C->Health->Health<=0.f || !GS || !GS->bRoundActive || bReloading || Magazine>=6 || Reserve<=0) return;
    bReloading = true; GS->Noise(ESpychoNoise::Reload, C->GetActorLocation(), 0.65f);
    GetWorld()->GetTimerManager().SetTimer(ReloadTimer, this, &USpychoHandgun::FinishReload, ReloadSeconds, false);
}
void USpychoHandgun::FinishReload()
{
    auto* C = Cast<ASpychoCharacter>(GetOwner()); auto* GS = GetWorld()->GetGameState<ASpychoGameState>();
    if (C && C->Health->Health>0.f && GS && GS->bRoundActive) SpychoRules::Reload(Magazine, Reserve, 6);
    bReloading = false;
}
void USpychoHandgun::CancelReload() { GetWorld()->GetTimerManager().ClearTimer(ReloadTimer); bReloading=false; }
void USpychoHandgun::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(USpychoHandgun, Magazine); DOREPLIFETIME(USpychoHandgun, Reserve); DOREPLIFETIME(USpychoHandgun, bReloading);
}
