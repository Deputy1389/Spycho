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
void USpychoHandgun::Fire(FRotator Aim, int32 PredictionKey)
{
    auto* C = Cast<ASpychoCharacter>(GetOwner());
    auto* GS = GetWorld()->GetGameState<ASpychoGameState>();
    if (!C || !C->HasAuthority() || Aim.ContainsNaN() || !SpychoRules::CanFire(Magazine, bReloading, C->Health->Health>0.f, GS && GS->CanShoot())) return;
    double Wait=NextShot-GetWorld()->GetTimeSeconds();
    if (Wait>0)
    {
        // Small arrival jitter must not discard a legitimate paced client click.
        // Debug/unpredicted spam does not get buffered; only one request can wait.
        if (PredictionKey>0 && Wait<=ShotInterval+.02 && !bBufferedShot)
        {
            bBufferedShot=true;
            FTimerDelegate D=FTimerDelegate::CreateUObject(this,&USpychoHandgun::FlushBufferedShot,Aim,PredictionKey);
            GetWorld()->GetTimerManager().SetTimer(BufferedShotTimer,D,float(Wait)+.001f,false);
        }
        return;
    }
    NextShot = GetWorld()->GetTimeSeconds() + ShotInterval; --Magazine;
    GS->NotifyHearing(ESpychoNoise::Gunshot,C->GetActorLocation(),2.8f,C);
    // Origin is always server-owned. Clients supply only a bounded unit aim rotation.
    Aim.Pitch = FMath::Clamp(FRotator::NormalizeAxis(Aim.Pitch), -85.f, 85.f);
    Aim.Roll = 0.f;
    C->Penetration->Fire(C->Camera->GetComponentLocation(), Aim.Vector());
    Fired(PredictionKey);
}
void USpychoHandgun::FlushBufferedShot(FRotator Aim,int32 PredictionKey)
{
    bBufferedShot=false; Fire(Aim,PredictionKey);
}
void USpychoHandgun::Fired_Implementation(int32 PredictionKey)
{
    if (auto* C = Cast<ASpychoCharacter>(GetOwner()))
    {
        C->ConfirmShotFeedback(PredictionKey);
    }
}
void USpychoHandgun::Reload()
{
    auto* C = Cast<ASpychoCharacter>(GetOwner()); auto* GS = GetWorld()->GetGameState<ASpychoGameState>();
    if (!C || !C->HasAuthority() || C->Health->Health<=0.f || !GS || !GS->bRoundActive || bReloading || Magazine>=6 || Reserve<=0) return;
    bReloading = true; GS->Noise(ESpychoNoise::Reload, C->GetActorLocation(), 0.65f);
    GS->NotifyHearing(ESpychoNoise::Reload,C->GetActorLocation(),.65f,C);
    GetWorld()->GetTimerManager().SetTimer(ReloadTimer, this, &USpychoHandgun::FinishReload, ReloadSeconds, false);
}
void USpychoHandgun::FinishReload()
{
    auto* C = Cast<ASpychoCharacter>(GetOwner()); auto* GS = GetWorld()->GetGameState<ASpychoGameState>();
    if (C && C->Health->Health>0.f && GS && GS->bRoundActive) SpychoRules::Reload(Magazine, Reserve, 6);
    bReloading = false;
}
void USpychoHandgun::CancelReload()
{
    GetWorld()->GetTimerManager().ClearTimer(ReloadTimer); GetWorld()->GetTimerManager().ClearTimer(BufferedShotTimer);
    bReloading=false; bBufferedShot=false;
}
void USpychoHandgun::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(USpychoHandgun, Magazine); DOREPLIFETIME(USpychoHandgun, Reserve); DOREPLIFETIME(USpychoHandgun, bReloading);
}
