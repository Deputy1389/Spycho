#include "SpychoHunterController.h"
#include "SpychoCharacter.h"
#include "SpychoHealth.h"
#include "SpychoHandgun.h"
#include "SpychoDoor.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

namespace
{
    const FVector Nodes[]={ {-550,-250,90},{-450,0,90},{-300,0,90},{-100,0,90},
        {-70,85,90},{-70,230,90},{200,270,90},{470,270,90},{470,85,90},
        {470,0,90},{470,-85,90},{470,-270,90},{200,-270,90},{-70,-270,90},
        {-70,-85,90},{200,0,90} };
    const FIntPoint Links[]={ {0,1},{1,2},{2,3},{3,4},{4,5},{5,6},{6,7},{7,8},{8,9},
        {9,10},{10,11},{11,12},{12,13},{13,14},{14,3},{3,15},{15,9} };
    int32 Closest(FVector Position)
    {
        int32 Best=0; float Distance=MAX_flt;
        for (int32 i=0;i<UE_ARRAY_COUNT(Nodes);++i)
        {
            float D=FVector::DistSquared2D(Position,Nodes[i]); if (D<Distance) { Distance=D; Best=i; }
        }
        return Best;
    }
}
ASpychoHunterController::ASpychoHunterController() { PrimaryActorTick.bCanEverTick=true; }
void ASpychoHunterController::HearNoise(ESpychoNoise Kind,FVector Location,float Gain,AActor* Source)
{
    auto* C=Cast<ASpychoCharacter>(GetPawn());
    if (!C || Source==C || C->Health->Health<=0 || Kind==ESpychoNoise::Creak || Kind==ESpychoNoise::Impact) return;
    float Radius=Kind==ESpychoNoise::Gunshot?5000.f:Gain*3400.f;
    FHitResult Hit; FCollisionQueryParams Q(SCENE_QUERY_STAT(SpychoBotHearing),false,C); if (Source) Q.AddIgnoredActor(Source);
    if (GetWorld()->LineTraceSingleByChannel(Hit,C->Camera->GetComponentLocation(),Location+FVector(0,0,40),ECC_Visibility,Q)) Radius*=.75f;
    if (FVector::Dist2D(C->GetActorLocation(),Location)>Radius) return;
    ++HeardEvents;
    float Now=GetWorld()->GetTimeSeconds();
    // Preserve a committed estimate through the reaction delay rather than
    // following every footstep with perfect aim through an opaque partition.
    if (MemoryUntil<Now || Now>AttackReady || Kind==ESpychoNoise::Gunshot)
    {
        LastKnown=Location+FVector(FMath::FRandRange(-65.f,65.f),FMath::FRandRange(-65.f,65.f),0); LastKnown.Z=115.f;
        AttackReady=Now+FMath::FRandRange(.75f,1.25f);
        MemoryUntil=Now+(Kind==ESpychoNoise::Gunshot?6.f:3.5f);
        PlanRoute(Closest(LastKnown));
    }
}
void ASpychoHunterController::PlanRoute(int32 Goal)
{
    APawn* C=GetPawn(); if (!C) return;
    Destination=Goal; int32 Start=Closest(C->GetActorLocation());
    int32 Previous[UE_ARRAY_COUNT(Nodes)]; for (int32& P:Previous) P=-1;
    TArray<int32> Queue;Queue.Add(Start);Previous[Start]=Start;
    for (int32 i=0;i<Queue.Num() && Previous[Goal]<0;++i)
        for (FIntPoint Edge:Links)
        {
            int32 Next=Edge.X==Queue[i]?Edge.Y:(Edge.Y==Queue[i]?Edge.X:-1);
            if (Next>=0 && Previous[Next]<0) { Previous[Next]=Queue[i]; Queue.Add(Next); }
        }
    Route.Empty();
    if (Previous[Goal]<0) return;
    for (int32 N=Goal;N!=Start;N=Previous[N]) Route.Insert(N,0);
    // Approach the graph before crossing any doorway, even when spawned off-node.
    Route.Insert(Start,0);
}
void ASpychoHunterController::Tick(float Dt)
{
    Super::Tick(Dt); auto* C=Cast<ASpychoCharacter>(GetPawn()); auto* GS=GetWorld()->GetGameState<ASpychoGameState>();
    if (!C || !HasAuthority() || C->Health->Health<=0 || !GS || !GS->bRoundActive) return;
    float Now=GetWorld()->GetTimeSeconds(); bool Visible=false;
    for (FConstPlayerControllerIterator It=GetWorld()->GetPlayerControllerIterator();It;++It)
    {
        auto* Target=Cast<ASpychoCharacter>(It->Get()->GetPawn()); if (!Target || Target->Health->Health<=0) continue;
        FVector Delta=Target->GetActorLocation()-C->GetActorLocation();
        if (Delta.Size2D()>1100 || FVector::DotProduct(C->GetActorForwardVector(),Delta.GetSafeNormal2D())<-.15f) continue;
        FHitResult Hit; FCollisionQueryParams Q(SCENE_QUERY_STAT(SpychoBotSight),false,C);
        GetWorld()->LineTraceSingleByChannel(Hit,C->Camera->GetComponentLocation(),Target->GetActorLocation()+FVector(0,0,25),ECC_Visibility,Q);
        if (Hit.GetActor()!=Target) continue;
        if (MemoryUntil<Now) AttackReady=Now+.7f;
        Visible=true; LastKnown=Target->GetActorLocation()+FVector(0,0,25);MemoryUntil=Now+2.f; break;
    }
    if (C->Handgun->Magazine==0) { C->Handgun->Reload(); return; }
    if (C->Handgun->bReloading) return;
    bool NearEstimate=MemoryUntil>Now && FVector::Dist2D(C->GetActorLocation(),LastKnown)<650.f;
    if (Visible || NearEstimate)
    {
        FRotator Aim=(LastKnown-C->Camera->GetComponentLocation()).Rotation();SetControlRotation(Aim);C->SetActorRotation(FRotator(0,Aim.Yaw,0));
        C->GetCharacterMovement()->StopMovementImmediately();
        if (Now>=AttackReady && Now>=NextAttack)
        {
            FVector Error=FVector(FMath::FRandRange(-20.f,20.f),FMath::FRandRange(-20.f,20.f),FMath::FRandRange(-12.f,12.f));
            int32 Before=C->Handgun->Magazine;C->Handgun->Fire((LastKnown+Error-C->Camera->GetComponentLocation()).Rotation());
            if (C->Handgun->Magazine<Before) ++ShotsTaken;
            NextAttack=Now+FMath::FRandRange(1.1f,1.7f);
            // One inferred wall shot, then move rather than emptying the magazine.
            if (!Visible) { MemoryUntil=0;PauseUntil=Now+.4f; }
        }
        return;
    }
    if (Now<PauseUntil) return;
    if (Route.IsEmpty())
    {
        const int32 Rooms[]={1,5,7,11,13};int32 Goal;
        do { Goal=Rooms[FMath::RandRange(0,UE_ARRAY_COUNT(Rooms)-1)]; } while (Goal==Destination);
        PlanRoute(Goal);
    }
    if (Route.IsEmpty()) return;
    FVector Delta=Nodes[Route[0]]-C->GetActorLocation();Delta.Z=0;
    if (Delta.Size()<35.f)
    {
        Route.RemoveAt(0); if (Route.IsEmpty()) PauseUntil=Now+FMath::FRandRange(.6f,1.8f);return;
    }
    FHitResult Hit;FCollisionQueryParams Q(SCENE_QUERY_STAT(SpychoBotDoor),false,C);
    GetWorld()->LineTraceSingleByChannel(Hit,C->GetActorLocation(),C->GetActorLocation()+Delta.GetSafeNormal()*140.f,ECC_Visibility,Q);
    if (auto* Door=Cast<ASpychoDoor>(Hit.GetActor()))
        if (!Door->bOpen) { Door->Toggle(true,C);PauseUntil=Now+.8f;return; }
    C->GetCharacterMovement()->MaxWalkSpeed=MemoryUntil>Now?160.f:135.f;
    SetControlRotation(Delta.Rotation());C->SetActorRotation(Delta.Rotation());C->AddMovementInput(Delta.GetSafeNormal());
}
