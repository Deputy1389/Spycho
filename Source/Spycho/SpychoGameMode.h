#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SpychoGameMode.generated.h"
class ASpychoCharacter;
UCLASS()
class SPYCHO_API ASpychoGameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    ASpychoGameMode();
    virtual void BeginPlay() override;
    virtual void PreLogin(const FString& Options,const FString& Address,const FUniqueNetIdRepl& Id,FString& Error) override;
    virtual void PostLogin(APlayerController* PC) override;
    virtual void Logout(AController* Exiting) override;
    void OnDeath(ASpychoCharacter* Victim);
    void ResetRound();
    void NetworkSmokeResult(bool Passed, bool Final);
private:
    FTimerHandle ResetTimer;
    FTimerHandle AmbientTimer;
    void Creak();
    void BeginSmokeTest();
    void SmokeCheckMovement(); void SmokeBeginAmmo();
    void SmokeCheckTrigger();
    void SmokeBufferedTrigger(); void SmokeCheckBuffer();
    void RunSmokeTest();
    bool bSmokeAmmoPassed = false;
    bool bSmokeMovementPassed = false;
    int32 SmokeFeedbackBefore = 0;
    FVector SmokeMovementStart;
    void BeginNetworkSmoke();
    bool bNetworkSmokeStarted = false;
    bool bNetworkSmokePassed = true;
    void CapturePrototype();
};
