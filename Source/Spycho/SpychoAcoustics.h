#pragma once
#include "CoreMinimal.h"
class UWorld;
class AActor;
struct FSpychoAcousticPath
{
    int32 Walls=0;
    bool bClosedDoor=false;
    bool bOpenRoute=false;
    FVector Arrival=FVector::ZeroVector;
    float Distance=0;
    float Transmission() const;
    float Cutoff() const;
};
namespace SpychoAcoustics
{
    int32 RoomAt(FVector Position);
    FSpychoAcousticPath Probe(UWorld* World,FVector Source,FVector Listener,AActor* Ignore=nullptr);
}
