#pragma once
#include "CoreMinimal.h"
#include "AIController.h"
#include "SpychoGameState.h"
#include "SpychoHunterController.generated.h"

enum class ESpychoHunterState : uint8 { Patrol, Listen, Investigate, Hold, Relocate };
// A small authored route graph, uncertain sound memory and explicit intentions.
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
    ESpychoHunterState State=ESpychoHunterState::Patrol;
    const TCHAR* StateName() const;
    bool ValidateRouteClearance() const;
private:
    friend class ASpychoGameMode;
    TArray<int32> Route;
    FVector LastKnown = FVector::ZeroVector;
    float MemoryUntil = 0.f;
    float AttackReady = 0.f;
    float NextAttack = 0.f;
    float PauseUntil = 0.f;
    bool bWallShotAllowed = false;
    float InvestigateAt=0.f;
    int32 SoundGoal=-1;
    float HoldYaw=0.f;
    bool bHadSight=false;
    ESpychoNoise RememberedKind=ESpychoNoise::Creak;
    TWeakObjectPtr<AActor> RememberedSource;
    int32 Destination = -1;
    void PlanRoute(int32 Goal);
};
