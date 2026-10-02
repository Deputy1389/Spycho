#include "SpychoGameMode.h"
#include "SpychoCharacter.h"
#include "SpychoGameState.h"
#include "SpychoPlayerController.h"
#include "SpychoHUD.h"
#include "SpychoDoor.h"
#include "SpychoHealth.h"
#include "SpychoHandgun.h"
#include "SpychoPenetration.h"
#include "Camera/CameraComponent.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

ASpychoGameMode::ASpychoGameMode()
{
    DefaultPawnClass=ASpychoCharacter::StaticClass(); PlayerControllerClass=ASpychoPlayerController::StaticClass();
    GameStateClass=ASpychoGameState::StaticClass(); HUDClass=ASpychoHUD::StaticClass();
}
void ASpychoGameMode::BeginPlay()
{
    Super::BeginPlay();
    GetWorldTimerManager().SetTimer(AmbientTimer,this,&ASpychoGameMode::Creak,23.f,false);
    if (FParse::Param(FCommandLine::Get(),TEXT("SpychoSmoke"))) { FTimerHandle T; GetWorldTimerManager().SetTimer(T,this,&ASpychoGameMode::BeginSmokeTest,4.f,false); }
}
void ASpychoGameMode::PreLogin(const FString& O,const FString& A,const FUniqueNetIdRepl& I,FString& E)
{
    Super::PreLogin(O,A,I,E); if (GetNumPlayers()>=2) E=TEXT("Spycho is a two-player duel. This house is full.");
}
void ASpychoGameMode::PostLogin(APlayerController* PC)
{
    Super::PostLogin(PC); GetWorldTimerManager().SetTimer(ResetTimer,this,&ASpychoGameMode::ResetRound,0.75f,false);
    if (GetNumPlayers()==2 && !bNetworkSmokeStarted && FParse::Param(FCommandLine::Get(),TEXT("SpychoNetSmoke")))
    {
        bNetworkSmokeStarted=true; FTimerHandle T; GetWorldTimerManager().SetTimer(T,this,&ASpychoGameMode::BeginNetworkSmoke,3.f,false);
    }
}
void ASpychoGameMode::BeginNetworkSmoke()
{
    for (FConstPlayerControllerIterator It=GetWorld()->GetPlayerControllerIterator();It;++It)
    {
        auto* PC=Cast<ASpychoPlayerController>(It->Get()); auto* C=PC?Cast<ASpychoCharacter>(PC->GetPawn()):nullptr;
        if (!C) continue;
        C->SetActorLocation(PC->IsLocalController()?FVector(310,320,90):FVector(0,320,90));
        if (!PC->IsLocalController()) PC->ClientSmokePrepare(GetGameState<ASpychoGameState>()->Round);
    }
    for (TActorIterator<ASpychoDoor> It(GetWorld());It;++It) { It->Toggle(false); break; }
}
void ASpychoGameMode::NetworkSmokeResult(bool Passed,bool Final)
{
    auto* GS=GetGameState<ASpychoGameState>();
    bool ServerOkay=GetNumPlayers()==2&&GS&&(Final?(GS->bRoundActive&&GS->Evidence.IsEmpty()):(!GS->bRoundActive&&GS->Evidence.Num()>=2));
    bNetworkSmokePassed &= Passed&&ServerOkay;
    UE_LOG(LogTemp,Display,TEXT("SPYCHO_NET_SERVER %s %s"),bNetworkSmokePassed?TEXT("PASS"):TEXT("FAIL"),Final?TEXT("final reset"):TEXT("remote authoritative shot"));
    if (!Final) return;
    for (FConstPlayerControllerIterator It=GetWorld()->GetPlayerControllerIterator();It;++It)
        if (auto* PC=Cast<ASpychoPlayerController>(It->Get())) if (!PC->IsLocalController()) PC->ClientSmokeFinish(bNetworkSmokePassed);
    FTimerHandle T; bool PassedAll=bNetworkSmokePassed;
    GetWorldTimerManager().SetTimer(T,FTimerDelegate::CreateLambda([PassedAll](){FPlatformMisc::RequestExitWithStatus(false,PassedAll?0:1);}),1.f,false);
}
void ASpychoGameMode::Logout(AController* Exiting)
{
    Super::Logout(Exiting); GetWorldTimerManager().SetTimer(ResetTimer,this,&ASpychoGameMode::ResetRound,1.f,false);
}
void ASpychoGameMode::ResetRound()
{
    GetWorldTimerManager().ClearTimer(ResetTimer);
    for (TActorIterator<ASpychoCharacter> It(GetWorld()); It; ++It) It->Destroy();
    for (TActorIterator<ASpychoDoor> It(GetWorld()); It; ++It) It->Reset();
    auto* GS=GetGameState<ASpychoGameState>(); if (!GS) return;
    ++GS->Round; GS->bRoundActive=true; GS->RoundMessage=GetNumPlayers()>1?TEXT("Duel"):TEXT("Practice patrol"); GS->OnRep_Round(); GS->ForceNetUpdate();
    int32 Index=0;
    for (FConstPlayerControllerIterator It=GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PC=It->Get(); if (!PC) continue;
        FTransform Spawn(Index==0?FRotator(0,90,0):FRotator(0,-90,0),Index==0?FVector(0,-820,90):FVector(350,520,90));
        RestartPlayerAtTransform(PC,Spawn); ++Index;
    }
    if (Index==1)
    {
        FActorSpawnParameters Params; Params.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
        auto* Bot=GetWorld()->SpawnActor<ASpychoCharacter>(FVector(310,320,90),FRotator(0,90,0),Params);
        Bot->bTestOpponent=true; Bot->Patrol={FVector(310,320,90),FVector(310,550,90),FVector(460,550,90),FVector(460,320,90)};
    }
}
void ASpychoGameMode::OnDeath(ASpychoCharacter* Victim)
{
    auto* GS=GetGameState<ASpychoGameState>(); if (!GS || !GS->bRoundActive) return;
    GS->bRoundActive=false; GS->RoundMessage=Victim->bTestOpponent?TEXT("The house is quiet. New round in 5 seconds."):TEXT("A hunter fell. New round in 5 seconds."); GS->ForceNetUpdate();
    for (TActorIterator<ASpychoCharacter> It(GetWorld()); It; ++It) It->Handgun->CancelReload();
    GetWorldTimerManager().SetTimer(ResetTimer,this,&ASpychoGameMode::ResetRound,5.f,false);
}
void ASpychoGameMode::Creak()
{
    if (auto* GS=GetGameState<ASpychoGameState>()) GS->Noise(ESpychoNoise::Creak,FVector(-480,400,240),0.055f);
    GetWorldTimerManager().SetTimer(AmbientTimer,this,&ASpychoGameMode::Creak,FMath::FRandRange(25.f,47.f),false);
}
void ASpychoGameMode::BeginSmokeTest()
{
    auto* PC=GetWorld()->GetFirstPlayerController(); auto* Shooter=PC?Cast<ASpychoCharacter>(PC->GetPawn()):nullptr;
    if (!Shooter) { FPlatformMisc::RequestExitWithStatus(false,1); return; }
    Shooter->Handgun->Fire(FRotator(85,0,0));
    Shooter->Handgun->Fire(FRotator(85,0,0));
    bSmokeAmmoPassed=Shooter->Handgun->Magazine==5;
    Shooter->Handgun->Reload(); Shooter->Handgun->Fire(FRotator(85,0,0));
    bSmokeAmmoPassed &= Shooter->Handgun->bReloading && Shooter->Handgun->Magazine==5;
    FTimerHandle T; GetWorldTimerManager().SetTimer(T,this,&ASpychoGameMode::RunSmokeTest,2.5f,false);
}
void ASpychoGameMode::RunSmokeTest()
{
    // Runs against the saved House map and real collision, damage and round systems.
    auto* PC=GetWorld()->GetFirstPlayerController(); auto* Shooter=PC?Cast<ASpychoCharacter>(PC->GetPawn()):nullptr;
    ASpychoCharacter* Bot=nullptr; for (TActorIterator<ASpychoCharacter> It(GetWorld()); It; ++It) if (It->bTestOpponent) Bot=*It;
    if (!Shooter || !Bot) { UE_LOG(LogTemp,Error,TEXT("SPYCHO_SMOKE FAIL missing pawns")); FPlatformMisc::RequestExitWithStatus(false,1); return; }
    bSmokeAmmoPassed &= Shooter->Handgun->Magazine==6 && Shooter->Handgun->Reserve==11 && !Shooter->Handgun->bReloading;
    UE_LOG(LogTemp,Display,TEXT("SPYCHO_SMOKE %s rate limit and timed reload"),bSmokeAmmoPassed?TEXT("PASS"):TEXT("FAIL"));
    Bot->Patrol.Empty(); Bot->SetActorLocation(FVector(310,320,90));
    Shooter->SetActorLocation(FVector(0,320,90));
    FVector Origin=Shooter->Camera->GetComponentLocation(); // hallway shot through x=120 drywall, away from doorway at y=450
    FRotator Aim=(Bot->GetActorLocation()-Origin).Rotation(); Shooter->Handgun->Fire(Aim);
    auto* GS=GetGameState<ASpychoGameState>();
    bool Pass=Bot->Health->Health<=0.f && Shooter->Handgun->Magazine==5 && !GS->bRoundActive && GS->Evidence.Num()>=2;
    UE_LOG(LogTemp,Display,TEXT("SPYCHO_SMOKE %s wall kill: HP=%.1f ammo=%d marks=%d roundActive=%d"),Pass?TEXT("PASS"):TEXT("FAIL"),Bot->Health->Health,Shooter->Handgun->Magazine,GS->Evidence.Num(),GS->bRoundActive);
    // A masonry exterior wall must stop exactly the same shot.
    GS->bRoundActive=true; Bot->Health->Health=100.f; Bot->SetActorLocation(FVector(740,320,90));
    Shooter->SetActorLocation(FVector(500,320,90)); Origin=Shooter->Camera->GetComponentLocation();
    Shooter->Penetration->Fire(Origin,(Bot->GetActorLocation()-Origin).GetSafeNormal());
    bool Blocked=Bot->Health->Health==100.f;
    UE_LOG(LogTemp,Display,TEXT("SPYCHO_SMOKE %s masonry stops bullet HP=%.1f"),Blocked?TEXT("PASS"):TEXT("FAIL"),Bot->Health->Health);
    ResetRound(); bool Cleared=GS->Evidence.IsEmpty();
    UE_LOG(LogTemp,Display,TEXT("SPYCHO_SMOKE %s round reset clears evidence"),Cleared?TEXT("PASS"):TEXT("FAIL"));
    bool AudioReady=GS->Sounds.Num()==8; for (const auto& S : GS->Sounds) AudioReady &= S!=nullptr;
    UE_LOG(LogTemp,Display,TEXT("SPYCHO_SMOKE %s all 8 sound references exist"),AudioReady?TEXT("PASS"):TEXT("FAIL"));
    bool DoorReady=false; for (TActorIterator<ASpychoDoor> It(GetWorld()); It; ++It) { It->Toggle(true); DoorReady=It->bOpen && It->bCareful; It->Reset(); break; }
    UE_LOG(LogTemp,Display,TEXT("SPYCHO_SMOKE %s door careful toggle/reset"),DoorReady?TEXT("PASS"):TEXT("FAIL"));
    FPlatformMisc::RequestExitWithStatus(false,(Pass&&Blocked&&Cleared&&bSmokeAmmoPassed&&AudioReady&&DoorReady)?0:1);
}
