#pragma once
#include "CoreMinimal.h"
#include "AIController.h"
#include "SpychoGameState.h"
#include "SpychoHunterController.generated.h"

// A small authored route graph and noisy last-known positions. No wall vision.
UCLASS()
class SPYCHO_API ASpychoHunterController : public AAIController
{
    GENERATED_BODY()
public:
    ASpychoHunterController();
    virtual void Tick(float DeltaSeconds) override;
    virtual void OnPossess(APawn* InPawn) override;
    void HearNoise(ESpychoNoise Kind,FVector Location,float Gain,AActor* Source);
    int32 ShotsTaken = 0;
    int32 HeardEvents = 0;
private:
    friend class ASpychoGameMode;
    TArray<int32> Route;
    FVector LastKnown = FVector::ZeroVector;
    float MemoryUntil = 0.f;
    float AttackReady = 0.f;
    float NextAttack = 0.f;
    float PauseUntil = 0.f;
    bool bWallShotAllowed = false;
    int32 Destination = -1;
    void PlanRoute(int32 Goal);
};
