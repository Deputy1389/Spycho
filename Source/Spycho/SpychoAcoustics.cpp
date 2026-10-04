#include "SpychoAcoustics.h"
#include "SpychoDoor.h"
#include "SpychoCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"
#include "SpychoHouseLayout.h"
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
    return SpychoHouse::RoomAt(P);
}
FSpychoAcousticPath SpychoAcoustics::Probe(UWorld* World,FVector Source,FVector Listener,AActor* Ignore)
{
    FSpychoAcousticPath Result;
    Result.Arrival=Source;Result.Distance=FVector::Dist(Source,Listener);
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
    // Keep direction approximate; this is an authored mansion portal model.
    int32 From=RoomAt(Source),To=RoomAt(Listener);
    if (Result.Walls>0 && From>=0 && To>=0 && From!=To)
    {
        constexpr int32 Count=SpychoHouse::RoomCount;
        bool Links[Count][Count]{};FVector Portals[Count][Count]{};
        auto Connect=[&](FVector Center,FVector Normal)
        {
            int32 A=RoomAt(Center+Normal*60),B=RoomAt(Center-Normal*60);
            if (A>=0&&B>=0&&A!=B) { Links[A][B]=Links[B][A]=true;Portals[A][B]=Portals[B][A]=Center; }
        };
        for (int32 I=0;I<UE_ARRAY_COUNT(SpychoHouse::OpenPortals);++I) Connect(SpychoHouse::OpenPortals[I],SpychoHouse::PortalNormals[I]);
        for (TActorIterator<ASpychoDoor> It(World);It;++It)
        {
            if (!It->bOpen || FMath::Abs(It->Swing->GetRelativeRotation().Yaw)<60.f) continue;
            FVector Center=It->GetActorTransform().TransformPosition(FVector(0,50.5f,154));
            Connect(Center,It->GetActorForwardVector());
        }
        TArray<int32> Queue{From};bool Visited[Count]{};Visited[From]=true;int32 Parent[Count];for (int32& P:Parent) P=-1;
        for (int32 i=0;i<Queue.Num();++i) for (int32 j=0;j<Count;++j)
            if (Links[Queue[i]][j]&&!Visited[j]) { Visited[j]=true;Parent[j]=Queue[i];Queue.Add(j); }
        Result.bOpenRoute=Visited[To];
        if (Result.bOpenRoute)
        {
            Result.Arrival=Portals[To][Parent[To]];FVector CursorPoint=Listener;float RouteLength=0;
            for (int32 Room=To;Room!=From;Room=Parent[Room])
            { FVector Portal=Portals[Room][Parent[Room]];RouteLength+=FVector::Dist(CursorPoint,Portal);CursorPoint=Portal; }
            Result.Distance=FMath::Max(Result.Distance,RouteLength+FVector::Dist(CursorPoint,Source));
        }
    }
    return Result;
}
