#include "SpychoPlayerController.h"
#include "SpychoGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "SpychoCharacter.h"
#include "SpychoGameState.h"
#include "SpychoHealth.h"
#include "SpychoHandgun.h"
#include "SpychoDoor.h"
#include "Camera/CameraComponent.h"
#include "EngineUtils.h"
#include "TimerManager.h"
void ASpychoPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    InputComponent->BindAction("Debug",IE_Pressed,this,&ASpychoPlayerController::ToggleDebug);
    InputComponent->BindAction("Restart",IE_Pressed,this,&ASpychoPlayerController::Restart);
}
void ASpychoPlayerController::ToggleDebug() { bDebug=!bDebug; }
void ASpychoPlayerController::Restart() { ServerRestart(); }
void ASpychoPlayerController::ServerRestart_Implementation()
{
    // Local/listen host alone controls the developer reset.
    if (IsLocalController()) if (auto* GM=GetWorld()->GetAuthGameMode<ASpychoGameMode>()) GM->ResetRound();
}
void ASpychoPlayerController::Host() { UGameplayStatics::OpenLevel(this,TEXT("/Game/Maps/House"),true,TEXT("listen")); }
void ASpychoPlayerController::Join(const FString& Address)
{
    if (!Address.IsEmpty() && Address.Len()<128 && !Address.Contains(TEXT("?"))) ClientTravel(Address,TRAVEL_Absolute);
}
void ASpychoPlayerController::ClientSmokePrepare_Implementation(int32 Round)
{
    SmokeRound=Round;
    FTimerHandle T; GetWorldTimerManager().SetTimer(T,this,&ASpychoPlayerController::SmokeFire,0.8f,false);
}
void ASpychoPlayerController::SmokeFire()
{
    if (auto* C=Cast<ASpychoCharacter>(GetPawn())) C->ServerFire((FVector(310,320,90)-C->Camera->GetComponentLocation()).Rotation());
    FTimerHandle T; GetWorldTimerManager().SetTimer(T,this,&ASpychoPlayerController::SmokeVerifyFirst,1.1f,false);
}
void ASpychoPlayerController::SmokeVerifyFirst()
{
    auto* GS=GetWorld()->GetGameState<ASpychoGameState>(); auto* C=Cast<ASpychoCharacter>(GetPawn());
    int32 Dead=0,Bots=0,Open=0;
    for (TActorIterator<ASpychoCharacter> It(GetWorld());It;++It) { Dead+=It->Health->Health<=0.f; Bots+=It->bTestOpponent; }
    for (TActorIterator<ASpychoDoor> It(GetWorld());It;++It) Open+=It->bOpen;
    bool Passed=GS&&C&&!GS->bRoundActive&&C->Handgun->Magazine==5&&GS->Evidence.Num()>=2&&Dead==1&&Bots==0&&Open>0;
    UE_LOG(LogTemp,Display,TEXT("SPYCHO_NET_CLIENT %s replicated wall kill/ammo/marks/round/door (dead=%d bots=%d open=%d)"),Passed?TEXT("PASS"):TEXT("FAIL"),Dead,Bots,Open);
    ServerSmokeResult(Passed,false);
    FTimerHandle T; GetWorldTimerManager().SetTimer(T,this,&ASpychoPlayerController::SmokeVerifyReset,5.2f,false);
}
void ASpychoPlayerController::SmokeVerifyReset()
{
    auto* GS=GetWorld()->GetGameState<ASpychoGameState>(); auto* C=Cast<ASpychoCharacter>(GetPawn());
    int32 Open=0; for (TActorIterator<ASpychoDoor> It(GetWorld());It;++It) Open+=It->bOpen;
    bool Passed=GS&&C&&GS->Round>SmokeRound&&GS->bRoundActive&&GS->Evidence.IsEmpty()&&C->Health->Health==100.f&&C->Handgun->Magazine==6&&C->Handgun->Reserve==12&&Open==0;
    UE_LOG(LogTemp,Display,TEXT("SPYCHO_NET_CLIENT %s replicated automatic reset"),Passed?TEXT("PASS"):TEXT("FAIL"));
    ServerSmokeResult(Passed,true);
}
void ASpychoPlayerController::ServerSmokeResult_Implementation(bool Passed,bool Final)
{
    if (FParse::Param(FCommandLine::Get(),TEXT("SpychoNetSmoke"))) if (auto* GM=GetWorld()->GetAuthGameMode<ASpychoGameMode>()) GM->NetworkSmokeResult(Passed,Final);
}
void ASpychoPlayerController::ClientSmokeFinish_Implementation(bool Passed) { FPlatformMisc::RequestExitWithStatus(false,Passed?0:1); }
