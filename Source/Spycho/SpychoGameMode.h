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
    void StartMatch();
    void NetworkSmokeResult(bool Passed, bool Final);
    void RoundTimeout();
private:
    FTimerHandle ResetTimer;
    FTimerHandle RoundTimer;
    int32 PlayedRounds=0;
    int32 ConsecutiveDraws=0;
    void BeginExperienceSmoke();
    void CheckExperienceSmoke();
    bool bExperiencePassed=true;
    FTimerHandle AmbientTimer;
    void Creak();
    void BeginSmokeTest();
    void SmokeCheckMovement(); void SmokeBeginAmmo();
    void SmokeCheckTrigger();
    void SmokeBufferedTrigger(); void SmokeCheckBuffer();
    void SmokeAimVolley(); void SmokeCheckVolley();
    int32 SmokeVolleyClicks = 0;
    void RunSmokeTest();
    void SmokeCheckDoorInput();
    bool bSmokeWorldPassed=false;
    bool bSmokeAmmoPassed = false;
    bool bSmokeMovementPassed = false;
    int32 SmokeFeedbackBefore = 0;
    FVector SmokeMovementStart;
    void BeginNetworkSmoke();
    bool bNetworkSmokeStarted = false;
    bool bNetworkSmokePassed = true;
    void CapturePrototype();
    void BeginBotSmoke(); void CheckBotSilence(); void CheckBotRoam(); void CheckBotWallShot(); void CheckBotDuel();
    void CheckBotWarning();
    bool bBotSmokePassed = true;
    void BeginPolishSmoke();void CheckPolishCoin();void CheckPolishSearch();void CheckPolishMatch();
    void CheckPolishRematch();
    bool bPolishPassed=true;
    FVector PolishEstimate;
};
