#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SpychoPlayerController.generated.h"
class USpychoOptions;
class USpychoMenu;
UCLASS()
class SPYCHO_API ASpychoPlayerController : public APlayerController
{
    GENERATED_BODY()
public:
    bool bDebug=false;
    bool bMenuOpen=false;
    bool bStartupMenu=false;
    UPROPERTY() TObjectPtr<USpychoOptions> Options;
    UPROPERTY() TObjectPtr<USpychoMenu> Menu;
    virtual void BeginPlay() override;
    virtual void Tick(float Dt) override;
    void ToggleMenu();
    void ShowMenu(bool Startup=false);
    void CloseMenu();
    void StartSolo();
    void ApplyOptions();
    void SaveOptions();
    void TestSound(bool Right);
    UFUNCTION(Server,Reliable) void ServerSetDifficulty(int32 Level);
    virtual void SetupInputComponent() override;
    UFUNCTION(Exec) void Host();
    UFUNCTION(Exec) void Join(const FString& Address);
    UFUNCTION(Server,Reliable) void ServerRestart();
    UFUNCTION(Client,Reliable) void ClientSmokePrepare(int32 Round);
    UFUNCTION(Server,Reliable) void ServerSmokeResult(bool Passed, bool Final);
    UFUNCTION(Client,Reliable) void ClientSmokeFinish(bool Passed);
private:
    void ToggleDebug(); void Restart(); void Rematch();
    void SmokeFire(); void SmokeVerifyFirst(); void SmokeVerifyReset();
    int32 SmokeRound=0;
};
