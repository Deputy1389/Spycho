#include "SpychoHunterController.h"
#include "SpychoCharacter.h"
#include "SpychoHealth.h"
#include "SpychoHandgun.h"
#include "SpychoDoor.h"
#include "SpychoAcoustics.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EngineUtils.h"
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
        int32 Best=0;float Distance=MAX_flt;
        for (int32 i=0;i<UE_ARRAY_COUNT(Nodes);++i)
            if (float D=FVector::DistSquared2D(Position,Nodes[i]);D<Distance) { Distance=D;Best=i; }
        return Best;
    }
}
ASpychoHunterController::ASpychoHunterController() { PrimaryActorTick.bCanEverTick=true; }
const TCHAR* ASpychoHunterController::StateName() const
{
    const TCHAR* Names[]={TEXT("patrol"),TEXT("listen"),TEXT("investigate"),TEXT("hold doorway"),TEXT("relocate")};
    return Names[int32(State)];
}
bool ASpychoHunterController::ValidateRouteClearance() const
{
    FCollisionQueryParams Q(SCENE_QUERY_STAT(SpychoRoutes),false);
    for (TActorIterator<ASpychoCharacter> It(GetWorld());It;++It) Q.AddIgnoredActor(*It);
    for (TActorIterator<ASpychoDoor> It(GetWorld());It;++It) Q.AddIgnoredActor(*It);
    bool Clear=true;
    for (auto Link:Links)
    {
        FHitResult H;
        if (GetWorld()->SweepSingleByChannel(H,Nodes[Link.X],Nodes[Link.Y],FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(30.f,88.f),Q))
        {
            Clear=false;UE_LOG(LogTemp,Warning,TEXT("SPYCHO_ROUTE blocked %d -> %d by %s"),Link.X,Link.Y,*GetNameSafe(H.GetActor()));
        }
    }
    return Clear;
}
void ASpychoHunterController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);PauseUntil=GetWorld()->GetTimeSeconds()+FMath::FRandRange(6.f,10.f);
    if (auto* C=Cast<ASpychoCharacter>(InPawn)) { C->bCareful=true;C->OnRep_Careful();HoldYaw=C->GetActorRotation().Yaw; }
}
void ASpychoHunterController::HearNoise(ESpychoNoise Kind,FVector Location,float Gain,AActor* Source)
{
    auto* C=Cast<ASpychoCharacter>(GetPawn());
    if (!C||Source==C||C->Health->Health<=0||Kind==ESpychoNoise::Creak||Kind==ESpychoNoise::Impact) return;
    auto Path=SpychoAcoustics::Probe(GetWorld(),Location+FVector(0,0,12),C->Camera->GetComponentLocation(),C);
    float Radius=(Kind==ESpychoNoise::Gunshot?5000.f:Gain*3400.f)*Path.Transmission();
    if (FVector::Dist2D(C->GetActorLocation(),Location)>Radius) return;
    ++HeardEvents;float Now=GetWorld()->GetTimeSeconds();
    bool NewDistraction=Kind==ESpychoNoise::Distraction&&(RememberedKind!=Kind||RememberedSource.Get()!=Source);
    if (MemoryUntil<Now||Kind==ESpychoNoise::Gunshot||NewDistraction)
    {
        RememberedKind=Kind;RememberedSource=Source;
        // An area estimate from an audible event, never the target's live position.
        float Error=Kind==ESpychoNoise::Distraction?65.f:(Path.Walls>0?140.f:70.f);
        LastKnown=Location+FVector(FMath::FRandRange(-Error,Error),FMath::FRandRange(-Error,Error),0);LastKnown.Z=115;
        SoundGoal=Closest(LastKnown);AttackReady=Now+FMath::FRandRange(2.f,3.f);MemoryUntil=Now+5.f;
        bWallShotAllowed=Kind==ESpychoNoise::Gunshot;
        InvestigateAt=Now+FMath::FRandRange(6.f,8.f);PauseUntil=InvestigateAt;
        State=ESpychoHunterState::Listen;HoldYaw=(LastKnown-C->GetActorLocation()).Rotation().Yaw;
        Route.Empty();C->GetCharacterMovement()->StopMovementImmediately();
    }
}
void ASpychoHunterController::PlanRoute(int32 Goal)
{
    APawn* C=GetPawn();if (!C||Goal<0||Goal>=UE_ARRAY_COUNT(Nodes)) return;
    Destination=Goal;int32 Start=Closest(C->GetActorLocation());
    int32 Previous[UE_ARRAY_COUNT(Nodes)];for (int32& P:Previous) P=-1;
    TArray<int32> Queue{Start};Previous[Start]=Start;
    for (int32 i=0;i<Queue.Num()&&Previous[Goal]<0;++i) for (FIntPoint Edge:Links)
    {
        int32 Next=Edge.X==Queue[i]?Edge.Y:(Edge.Y==Queue[i]?Edge.X:-1);
        if (Next>=0&&Previous[Next]<0) { Previous[Next]=Queue[i];Queue.Add(Next); }
    }
    Route.Empty();if (Previous[Goal]<0) return;
    for (int32 N=Goal;N!=Start;N=Previous[N]) Route.Insert(N,0);
    Route.Insert(Start,0);
}
void ASpychoHunterController::Tick(float Dt)
{
    Super::Tick(Dt);auto* C=Cast<ASpychoCharacter>(GetPawn());auto* GS=GetWorld()->GetGameState<ASpychoGameState>();
    if (!C||!HasAuthority()||C->Health->Health<=0||!GS||!GS->bRoundActive) return;
    float Now=GetWorld()->GetTimeSeconds();bool Visible=false;
    for (FConstPlayerControllerIterator It=GetWorld()->GetPlayerControllerIterator();It;++It)
    {
        auto* Target=Cast<ASpychoCharacter>(It->Get()->GetPawn());if (!Target||Target->Health->Health<=0) continue;
        FVector Delta=Target->GetActorLocation()-C->GetActorLocation();
        if (Delta.Size2D()>1100||FVector::DotProduct(C->GetActorForwardVector(),Delta.GetSafeNormal2D())<.65f) continue;
        FHitResult Hit;FCollisionQueryParams Q(SCENE_QUERY_STAT(SpychoBotSight),false,C);
        GetWorld()->LineTraceSingleByChannel(Hit,C->Camera->GetComponentLocation(),Target->GetActorLocation()+FVector(0,0,25),ECC_Visibility,Q);
        if (Hit.GetActor()!=Target) continue;
        GetWorld()->LineTraceSingleByChannel(Hit,C->Camera->GetComponentLocation(),Target->Camera->GetComponentLocation(),ECC_Visibility,Q);
        if (Hit.GetActor()!=Target) continue;
        if (!bHadSight) AttackReady=Now+1.2f;
        Visible=true;LastKnown=Target->GetActorLocation()+FVector(0,0,25);MemoryUntil=Now+2.f;bWallShotAllowed=false;SoundGoal=-1;break;
    }
    if (bHadSight&&!Visible) { State=ESpychoHunterState::Hold;PauseUntil=Now+3.f;HoldYaw=C->GetActorRotation().Yaw; }
    bHadSight=Visible;
    if (C->Handgun->Magazine==0) { C->Handgun->Reload();return; }
    if (C->Handgun->bReloading) return;
    if (Visible||(MemoryUntil>Now&&State==ESpychoHunterState::Listen))
    {
        FRotator Aim=(LastKnown-C->Camera->GetComponentLocation()).Rotation();
        SetControlRotation(FMath::RInterpTo(GetControlRotation(),Aim,Dt,6.f));C->SetActorRotation(FRotator(0,GetControlRotation().Yaw,0));
        C->GetCharacterMovement()->StopMovementImmediately();
        if ((Visible||bWallShotAllowed)&&Now>=AttackReady&&Now>=NextAttack)
        {
            FVector Error=Visible?FVector(FMath::FRandRange(-14.f,14.f),FMath::FRandRange(-14.f,14.f),FMath::FRandRange(-8.f,8.f)):FVector::ZeroVector;
            int32 Before=C->Handgun->Magazine;C->Handgun->Fire((LastKnown+Error-C->Camera->GetComponentLocation()).Rotation());
            if (C->Handgun->Magazine<Before) ++ShotsTaken;
            NextAttack=Now+FMath::FRandRange(1.8f,2.8f);
            if (!Visible)
            {
                MemoryUntil=0;bWallShotAllowed=false;SoundGoal=-1;State=ESpychoHunterState::Relocate;
                PauseUntil=Now+FMath::FRandRange(2.5f,4.f);Route.Empty();
            }
        }
        return;
    }
    if (Now<PauseUntil)
    {
        C->GetCharacterMovement()->StopMovementImmediately();
        float Scan=HoldYaw+FMath::Sin(Now*.65f)*18.f;
        SetControlRotation(FMath::RInterpTo(GetControlRotation(),FRotator(0,Scan,0),Dt,2.f));C->SetActorRotation(FRotator(0,GetControlRotation().Yaw,0));return;
    }
    if (State==ESpychoHunterState::Listen&&SoundGoal>=0&&Now>=InvestigateAt)
    {
        PlanRoute(SoundGoal);SoundGoal=-1;State=ESpychoHunterState::Investigate;MemoryUntil=0;bWallShotAllowed=false;
    }
    if (Route.IsEmpty())
    {
        const int32 Rooms[]={1,5,7,11,13};int32 Goal;
        do { Goal=Rooms[FMath::RandRange(0,UE_ARRAY_COUNT(Rooms)-1)]; } while (Goal==Destination||Goal==Closest(C->GetActorLocation()));
        PlanRoute(Goal);State=ESpychoHunterState::Patrol;
    }
    if (Route.IsEmpty()) return;
    FVector Delta=Nodes[Route[0]]-C->GetActorLocation();Delta.Z=0;
    if (Delta.Size()<32.f)
    {
        int32 Arrived=Route[0];Route.RemoveAt(0);
        if (Route.IsEmpty()) { State=ESpychoHunterState::Hold;PauseUntil=Now+FMath::FRandRange(4.f,8.f);HoldYaw=C->GetActorRotation().Yaw; }
        else if (Arrived==2||Arrived==4||Arrived==8||Arrived==10||Arrived==14)
        { PauseUntil=Now+FMath::FRandRange(.7f,1.3f);HoldYaw=(Nodes[Route[0]]-C->GetActorLocation()).Rotation().Yaw; }
        return;
    }
    FHitResult Hit;FCollisionQueryParams Q(SCENE_QUERY_STAT(SpychoBotDoor),false,C);
    GetWorld()->LineTraceSingleByChannel(Hit,C->GetActorLocation(),C->GetActorLocation()+Delta.GetSafeNormal()*140.f,ECC_Visibility,Q);
    if (auto* Door=Cast<ASpychoDoor>(Hit.GetActor())) if (!Door->bOpen)
    {
        Door->Toggle(true,C);PauseUntil=Now+2.f;HoldYaw=Delta.Rotation().Yaw;return;
    }
    C->GetCharacterMovement()->MaxWalkSpeed=75.f;
    SetControlRotation(FMath::RInterpConstantTo(GetControlRotation(),Delta.Rotation(),Dt,130.f));C->SetActorRotation(FRotator(0,GetControlRotation().Yaw,0));
    C->AddMovementInput(Delta.GetSafeNormal());
}
