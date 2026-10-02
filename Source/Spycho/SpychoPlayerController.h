#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SpychoPlayerController.generated.h"
UCLASS()
class SPYCHO_API ASpychoPlayerController : public APlayerController
{
    GENERATED_BODY()
public:
    bool bDebug=false;
    virtual void SetupInputComponent() override;
    UFUNCTION(Exec) void Host();
    UFUNCTION(Exec) void Join(const FString& Address);
    UFUNCTION(Server,Reliable) void ServerRestart();
    UFUNCTION(Client,Reliable) void ClientSmokePrepare(int32 Round);
    UFUNCTION(Server,Reliable) void ServerSmokeResult(bool Passed, bool Final);
    UFUNCTION(Client,Reliable) void ClientSmokeFinish(bool Passed);
private:
    void ToggleDebug(); void Restart();
    void SmokeFire(); void SmokeVerifyFirst(); void SmokeVerifyReset();
    int32 SmokeRound=0;
};
