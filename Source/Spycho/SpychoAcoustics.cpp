#include "SpychoAcoustics.h"
#include "SpychoDoor.h"
#include "SpychoCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"
float FSpychoAcousticPath::Transmission() const
{
    return Walls==0?1.f:(bOpenRoute?.82f:FMath::Pow(.68f,Walls)*(bClosedDoor?.8f:1.f));
}
float FSpychoAcousticPath::Cutoff() const
{
    return Walls==0?18000.f:(bOpenRoute?6500.f:FMath::Max(900.f,(bClosedDoor?2100.f:3200.f)/Walls));
}
int32 SpychoAcoustics::RoomAt(FVector P)
{
    if (P.X<-700 || P.X>700 || FMath::Abs(P.Y)>450) return -1;
    if (P.X<-300) return 0;
    if (FMath::Abs(P.Y)<85) return 1;
    return P.Y>0?(P.X<200?2:3):(P.X<200?4:5);
}
FSpychoAcousticPath SpychoAcoustics::Probe(UWorld* World,FVector Source,FVector Listener,AActor* Ignore)
{
    FSpychoAcousticPath Result;
    FCollisionQueryParams Q(SCENE_QUERY_STAT(SpychoAcousticPath),false,Ignore);
    FVector Cursor=Source;
    float LastBarrier=-100.f;
    for (int32 i=0;i<10 && Result.Walls<4;++i)
    {
        FHitResult H;
        if (!World->LineTraceSingleByChannel(H,Cursor,Listener,ECC_Visibility,Q) || !H.GetActor()) break;
        Q.AddIgnoredActor(H.GetActor());
        if (!Cast<ASpychoCharacter>(H.GetActor()) && FVector::DistSquared(H.ImpactPoint,Source)>20.f*20.f)
        {
            float Distance=FVector::Dist(Source,H.ImpactPoint);
            if (Distance-LastBarrier>20.f) ++Result.Walls;
            LastBarrier=Distance;
            if (auto* Door=Cast<ASpychoDoor>(H.GetActor())) Result.bClosedDoor|=!Door->bOpen;
        }
        Cursor=H.ImpactPoint+(Listener-H.ImpactPoint).GetSafeNormal()*2.f;
    }
    // Open doors connect rooms even when the straight path crosses a partition.
    // Keep direction approximate; this is a small-house portal approximation.
    int32 From=RoomAt(Source),To=RoomAt(Listener);
    if (Result.Walls>0 && From>=0 && To>=0 && From!=To)
    {
        bool Links[6][6]{};
        for (TActorIterator<ASpychoDoor> It(World);It;++It)
        {
            if (!It->bOpen || FMath::Abs(It->Swing->GetRelativeRotation().Yaw)<60.f) continue;
            FVector Center=It->GetActorTransform().TransformPosition(FVector(0,50.5f,154));
            FVector Normal=It->GetActorForwardVector()*60.f;
            int32 A=RoomAt(Center+Normal),B=RoomAt(Center-Normal);
            if (A>=0 && B>=0) Links[A][B]=Links[B][A]=true;
        }
        TArray<int32> Queue{From};bool Visited[6]{};Visited[From]=true;
        for (int32 i=0;i<Queue.Num();++i) for (int32 j=0;j<6;++j)
            if (Links[Queue[i]][j]&&!Visited[j]) { Visited[j]=true;Queue.Add(j); }
        Result.bOpenRoute=Visited[To];
    }
    return Result;
}
