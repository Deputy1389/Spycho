#include "SpychoGameMode.h"
#include "SpychoCharacter.h"
#include "SpychoGameState.h"
#include "SpychoPlayerController.h"
#include "SpychoHUD.h"
#include "SpychoDoor.h"
#include "SpychoDistraction.h"
#include "SpychoAcoustics.h"
#include "SpychoHealth.h"
#include "SpychoHandgun.h"
#include "SpychoPenetration.h"
#include "Camera/CameraComponent.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "UnrealClient.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "InputKeyEventArgs.h"
#include "SpychoHunterController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraActor.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/StaticMeshComponent.h"

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
    if (FParse::Param(FCommandLine::Get(),TEXT("SpychoCapture"))) { FTimerHandle T; GetWorldTimerManager().SetTimer(T,this,&ASpychoGameMode::CapturePrototype,5.f,false); }
    if (FParse::Param(FCommandLine::Get(),TEXT("SpychoBotSmoke"))) { FTimerHandle T; GetWorldTimerManager().SetTimer(T,this,&ASpychoGameMode::BeginBotSmoke,4.f,false); }
    if (FParse::Param(FCommandLine::Get(),TEXT("SpychoPolishSmoke"))) { FTimerHandle T;GetWorldTimerManager().SetTimer(T,this,&ASpychoGameMode::BeginPolishSmoke,4.f,false); }
}
void ASpychoGameMode::CapturePrototype()
{
    auto* PC=GetWorld()->GetFirstPlayerController(); if (!PC || !PC->GetPawn()) return;
    bool AimCapture=FParse::Param(FCommandLine::Get(),TEXT("SpychoCaptureAim"));
    PC->SetIgnoreMoveInput(true); PC->SetIgnoreLookInput(true);
    if (AimCapture)
    {
        PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::RightMouseButton,IE_Pressed,1.f));
        FTimerHandle Freeze; GetWorldTimerManager().SetTimer(Freeze,FTimerDelegate::CreateLambda([PC](){PC->GetPawn()->DisableInput(PC);}),.1f,false);
    }
    else PC->GetPawn()->DisableInput(PC);
    bool PlanCapture=FParse::Param(FCommandLine::Get(),TEXT("SpychoCapturePlan"));
    bool BotCapture=FParse::Param(FCommandLine::Get(),TEXT("SpychoCaptureBot"));
    bool LoungeCapture=FParse::Param(FCommandLine::Get(),TEXT("SpychoCaptureLounge"));
    bool DoorCapture=FParse::Param(FCommandLine::Get(),TEXT("SpychoCaptureDoor"));
    bool ReloadCapture=FParse::Param(FCommandLine::Get(),TEXT("SpychoCaptureReload"));
    bool FireCapture=FParse::Param(FCommandLine::Get(),TEXT("SpychoCaptureFire"));
    PC->GetPawn()->SetActorLocation(LoungeCapture?FVector(-420,-100,90):(BotCapture?FVector(350,130,90):FVector(-220,0,90)));
    PC->SetControlRotation(FRotator(0,LoungeCapture?112.f:(BotCapture?90.f:0.f),0));
    if (DoorCapture) { PC->GetPawn()->SetActorLocation(FVector(-440,0,90));PC->SetControlRotation(FRotator::ZeroRotator); }
    if (ReloadCapture)
    {
        auto* C=Cast<ASpychoCharacter>(PC->GetPawn());C->Handgun->Magazine=3;C->ServerReload();
    }
    if (FireCapture)
    {
        FTimerHandle Shot;GetWorldTimerManager().SetTimer(Shot,FTimerDelegate::CreateLambda([PC]()
        { auto* C=Cast<ASpychoCharacter>(PC->GetPawn());C->ServerFire(PC->GetControlRotation(),0); }),.94f,false);
    }
    if (BotCapture) for (TActorIterator<ASpychoCharacter> It(GetWorld());It;++It) if (It->bTestOpponent)
    {
        It->SetActorLocation(FVector(350,310,90));It->SetActorRotation(FRotator(0,-90,0));if (It->GetController()) It->GetController()->SetControlRotation(FRotator(0,-90,0));
    }
    if (PlanCapture)
    {
        for (TActorIterator<AActor> It(GetWorld());It;++It) if (It->ActorHasTag(TEXT("CaptureCeiling"))) It->SetActorHiddenInGame(true);
        auto* View=GetWorld()->SpawnActor<ACameraActor>(FVector(0,0,1600),FRotator(-90,90,0));auto* Lens=View->GetCameraComponent();
        Lens->ProjectionMode=ECameraProjectionMode::Orthographic;Lens->OrthoWidth=1760.f;
        Lens->PostProcessSettings.bOverride_AutoExposureBias=true;Lens->PostProcessSettings.AutoExposureBias=0.f;
        // A presentation-only fill makes the roofless layout legible. Normal
        // gameplay captures retain the authored dark interior lighting.
        auto* Fill=GetWorld()->SpawnActor<ADirectionalLight>(FVector(0,0,900),FRotator(-90,0,0));
        auto* Light=Cast<UDirectionalLightComponent>(Fill->GetLightComponent());
        Light->SetMobility(EComponentMobility::Movable);Light->SetIntensity(2.f);Light->SetCastShadows(false);
        PC->SetViewTarget(View);if (PC->GetHUD()) PC->GetHUD()->bShowHUD=false;
    }
    FString Directory=FPaths::ProjectSavedDir()/TEXT("Screenshots"); IFileManager::Get().MakeDirectory(*Directory,true);
    FTimerHandle T;
    GetWorldTimerManager().SetTimer(T,FTimerDelegate::CreateLambda([this,Directory,AimCapture,PlanCapture,BotCapture,LoungeCapture,DoorCapture,ReloadCapture,FireCapture]()
    {
        const TCHAR* Name=ReloadCapture?TEXT("SpychoReload.png"):(DoorCapture?TEXT("SpychoDoor.png"):(PlanCapture?TEXT("SpychoPlan.png"):(BotCapture?TEXT("SpychoBot.png"):(LoungeCapture?TEXT("SpychoLounge.png"):(AimCapture?TEXT("SpychoAim.png"):TEXT("Spycho.png"))))));
        FScreenshotRequest::RequestScreenshot(Directory/(FireCapture?TEXT("SpychoFire.png"):Name),false,false);
        FTimerHandle Exit; GetWorldTimerManager().SetTimer(Exit,FTimerDelegate::CreateLambda([](){FPlatformMisc::RequestExitWithStatus(false,0);}),2.f,false);
    }),1.f,false);
}
void ASpychoGameMode::PreLogin(const FString& O,const FString& A,const FUniqueNetIdRepl& I,FString& E)
{
    Super::PreLogin(O,A,I,E); if (GetNumPlayers()>=2) E=TEXT("Spycho is a two-player duel. This house is full.");
}
void ASpychoGameMode::PostLogin(APlayerController* PC)
{
    Super::PostLogin(PC); GetWorldTimerManager().SetTimer(ResetTimer,this,&ASpychoGameMode::StartMatch,0.75f,false);
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
        C->SetActorLocation(PC->IsLocalController()?FVector(350,260,90):FVector(350,0,90));
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
    Super::Logout(Exiting); GetWorldTimerManager().SetTimer(ResetTimer,this,&ASpychoGameMode::StartMatch,1.f,false);
}
void ASpychoGameMode::ResetRound()
{
    GetWorldTimerManager().ClearTimer(ResetTimer);
    for (TActorIterator<ASpychoCharacter> It(GetWorld()); It; ++It)
    {
        if (auto* Hunter=Cast<ASpychoHunterController>(It->GetController())) Hunter->Destroy();
        It->Destroy();
    }
    for (TActorIterator<ASpychoDoor> It(GetWorld()); It; ++It) It->Reset();
    for (TActorIterator<ASpychoDistraction> It(GetWorld());It;++It) It->Destroy();
    auto* GS=GetGameState<ASpychoGameState>(); if (!GS) return;
    ++GS->Round; GS->bRoundActive=true; GS->RoundMessage=TEXT("First to three. Listen carefully."); GS->OnRep_Round(); GS->ForceNetUpdate();
    const FVector Starts[][2]={{{-560,-300,90},{530,300,90}},{{-190,160,90},{530,-130,90}},{{-230,-160,90},{340,360,90}}};
    int32 MatchRound=GS->ScoreA+GS->ScoreB;int32 Pair=(MatchRound/2)%3;
    auto SpawnFor=[&](int32 Slot)
    {
        FVector P=Starts[Pair][(Slot+MatchRound)%2];
        return FTransform((FVector(0,0,90)-P).Rotation(),P);
    };
    int32 Index=0;
    for (FConstPlayerControllerIterator It=GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PC=It->Get(); if (!PC) continue;
        RestartPlayerAtTransform(PC,SpawnFor(Index));
        if (auto* C=Cast<ASpychoCharacter>(PC->GetPawn())) C->DuelSlot=Index;
        ++Index;
    }
    if (Index==1)
    {
        FActorSpawnParameters Params; Params.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
        auto* Bot=GetWorld()->SpawnActor<ASpychoCharacter>(ASpychoCharacter::StaticClass(),SpawnFor(1),Params);Bot->bTestOpponent=true;Bot->DuelSlot=1;
        auto* Hunter=GetWorld()->SpawnActor<ASpychoHunterController>();Hunter->Possess(Bot);
        if (FParse::Param(FCommandLine::Get(),TEXT("SpychoSmoke")) || FParse::Param(FCommandLine::Get(),TEXT("SpychoCapture")) || FParse::Param(FCommandLine::Get(),TEXT("SpychoBotSmoke")) || FParse::Param(FCommandLine::Get(),TEXT("SpychoPolishSmoke"))) Hunter->SetActorTickEnabled(false);
    }
}
void ASpychoGameMode::StartMatch()
{
    if (auto* GS=GetGameState<ASpychoGameState>()) { GS->ScoreA=0;GS->ScoreB=0;GS->bMatchOver=false; }
    ResetRound();
}
void ASpychoGameMode::OnDeath(ASpychoCharacter* Victim)
{
    auto* GS=GetGameState<ASpychoGameState>(); if (!GS || !GS->bRoundActive) return;
    GS->bRoundActive=false;
    if (Victim->DuelSlot==1) ++GS->ScoreA;else ++GS->ScoreB;
    GS->bMatchOver=GS->ScoreA>=3||GS->ScoreB>=3;
    GS->RoundMessage=GS->bMatchOver?TEXT("Match complete. Enter to rematch."):TEXT("Round over. Switching positions in 5 seconds.");GS->ForceNetUpdate();
    for (TActorIterator<ASpychoCharacter> It(GetWorld()); It; ++It) It->Handgun->CancelReload();
    if (!GS->bMatchOver) GetWorldTimerManager().SetTimer(ResetTimer,this,&ASpychoGameMode::ResetRound,5.f,false);
}
void ASpychoGameMode::Creak()
{
    if (auto* GS=GetGameState<ASpychoGameState>()) GS->Noise(ESpychoNoise::Creak,FVector(-480,400,240),0.055f);
    GetWorldTimerManager().SetTimer(AmbientTimer,this,&ASpychoGameMode::Creak,FMath::FRandRange(25.f,47.f),false);
}
void ASpychoGameMode::BeginSmokeTest()
{
    auto* PC=GetWorld()->GetFirstPlayerController(); if (!PC || !PC->GetPawn()) { FPlatformMisc::RequestExitWithStatus(false,1); return; }
    PC->GetPawn()->SetActorLocation(FVector(-220,0,90)); PC->SetControlRotation(FRotator(0,0,0));
    SmokeMovementStart=PC->GetPawn()->GetActorLocation();
    PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::LeftShift,IE_Pressed,1.f));
    PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::W,IE_Pressed,1.f));
    FTimerHandle T; GetWorldTimerManager().SetTimer(T,this,&ASpychoGameMode::SmokeCheckMovement,.6f,false);
}
void ASpychoGameMode::SmokeCheckMovement()
{
    auto* PC=GetWorld()->GetFirstPlayerController(); auto* C=PC?Cast<ASpychoCharacter>(PC->GetPawn()):nullptr;
    if (!C) { FPlatformMisc::RequestExitWithStatus(false,1); return; }
    PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::W,IE_Released,0.f));
    float Travel=C->GetActorLocation().X-SmokeMovementStart.X;
    bSmokeMovementPassed=Travel>70.f&&Travel<250.f&&C->bSprinting&&C->GetCharacterMovement()->MaxWalkSpeed==420.f;
    UE_LOG(LogTemp,Display,TEXT("SPYCHO_SMOKE %s actual W + Shift sprint grounded movement %.1f cm"),bSmokeMovementPassed?TEXT("PASS"):TEXT("FAIL"),Travel);
    PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::LeftShift,IE_Released,0.f));
    PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::LeftAlt,IE_Pressed,1.f));
    PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::LeftControl,IE_Pressed,1.f));
    PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::MouseY,IE_Axis,8.f));
    PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::MouseX,IE_Axis,8.f));
    PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::RightMouseButton,IE_Pressed,1.f));
    FTimerHandle T; GetWorldTimerManager().SetTimer(T,this,&ASpychoGameMode::SmokeBeginAmmo,.4f,false);
}
void ASpychoGameMode::SmokeBeginAmmo()
{
    auto* PC=GetWorld()->GetFirstPlayerController(); auto* Shooter=PC?Cast<ASpychoCharacter>(PC->GetPawn()):nullptr;
    if (!Shooter) { FPlatformMisc::RequestExitWithStatus(false,1); return; }
    bool Mouse=FRotator::NormalizeAxis(PC->GetControlRotation().Pitch)>0.f && PC->GetControlRotation().Yaw>0.f;
    FTransform GunTransform=Shooter->Gun->GetComponentTransform();
    FVector Sight=Shooter->Camera->GetComponentTransform().InverseTransformPosition(GunTransform.TransformPosition(FVector(0,-3,12.6f)));
    float Alignment=FVector::DotProduct(GunTransform.TransformVectorNoScale(FVector::YAxisVector),Shooter->Camera->GetForwardVector());
    bool Sights=Shooter->GetAimAlpha()>.98f && Alignment>.995f && FMath::Abs(Sight.Y)<1.f && FMath::Abs(Sight.Z)<1.f && Sight.X>25.f && Sight.X<65.f;
    UE_LOG(LogTemp,Display,TEXT("SPYCHO_SMOKE %s raw MouseY up/MouseX right (pitch %.2f yaw %.2f)"),Mouse?TEXT("PASS"):TEXT("FAIL"),PC->GetControlRotation().Pitch,PC->GetControlRotation().Yaw);
    UE_LOG(LogTemp,Display,TEXT("SPYCHO_SMOKE %s smooth aligned aiming alpha %.3f"),Sights?TEXT("PASS"):TEXT("FAIL"),Shooter->GetAimAlpha());
    bSmokeMovementPassed &= Mouse&&Sights;
    bool Stance=Shooter->bCareful&&!Shooter->bIsCrouched&&!Shooter->bSprinting&&Shooter->GetCharacterMovement()->MaxWalkSpeed==95.f;
    UE_LOG(LogTemp,Display,TEXT("SPYCHO_SMOKE %s actual Ctrl slow walk with standing eye height"),Stance?TEXT("PASS"):TEXT("FAIL")); bSmokeMovementPassed &= Stance;
    PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::LeftAlt,IE_Released,0.f));
    PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::LeftControl,IE_Released,0.f));
    SmokeFeedbackBefore=Shooter->GetShotFeedbackCount();
    PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::LeftMouseButton,IE_Pressed,1.f));
    FTimerHandle T; GetWorldTimerManager().SetTimer(T,this,&ASpychoGameMode::SmokeCheckTrigger,.05f,false);
}
void ASpychoGameMode::SmokeCheckTrigger()
{
    auto* PC=GetWorld()->GetFirstPlayerController(); auto* Shooter=PC?Cast<ASpychoCharacter>(PC->GetPawn()):nullptr;
    if (!Shooter) { FPlatformMisc::RequestExitWithStatus(false,1); return; }
    bool Responsive=Shooter->Handgun->Magazine==5&&Shooter->GetShotFeedbackCount()==SmokeFeedbackBefore+1;
    UE_LOG(LogTemp,Display,TEXT("SPYCHO_SMOKE %s actual trigger immediate single feedback/ammo"),Responsive?TEXT("PASS"):TEXT("FAIL"));
    bSmokeMovementPassed &= Responsive;
    PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::LeftMouseButton,IE_Released,0.f));
    Shooter->Handgun->Fire(FRotator(85,0,0));
    bSmokeAmmoPassed=Shooter->Handgun->Magazine==5;
    FTimerHandle T; GetWorldTimerManager().SetTimer(T,this,&ASpychoGameMode::SmokeBufferedTrigger,.03f,false);
}
void ASpychoGameMode::SmokeBufferedTrigger()
{
    auto* PC=GetWorld()->GetFirstPlayerController();
    if (!PC || !PC->GetPawn()) { FPlatformMisc::RequestExitWithStatus(false,1); return; }
    PC->SetControlRotation(FRotator(85,0,0));
    PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::LeftMouseButton,IE_Pressed,1.f));
    FTimerHandle T; GetWorldTimerManager().SetTimer(T,this,&ASpychoGameMode::SmokeCheckBuffer,.13f,false);
}
void ASpychoGameMode::SmokeCheckBuffer()
{
    auto* PC=GetWorld()->GetFirstPlayerController(); auto* Shooter=PC?Cast<ASpychoCharacter>(PC->GetPawn()):nullptr;
    if (!Shooter) { FPlatformMisc::RequestExitWithStatus(false,1); return; }
    PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::LeftMouseButton,IE_Released,0.f));
    bool Buffered=Shooter->Handgun->Magazine==4&&Shooter->GetShotFeedbackCount()==SmokeFeedbackBefore+2;
    UE_LOG(LogTemp,Display,TEXT("SPYCHO_SMOKE %s early trigger queued once while holding ADS"),Buffered?TEXT("PASS"):TEXT("FAIL"));
    bSmokeAmmoPassed &= Buffered;
    FTimerHandle T;GetWorldTimerManager().SetTimer(T,this,&ASpychoGameMode::SmokeAimVolley,.17f,false);
}
void ASpychoGameMode::SmokeAimVolley()
{
    auto* PC=GetWorld()->GetFirstPlayerController();auto* C=PC?Cast<ASpychoCharacter>(PC->GetPawn()):nullptr;
    if (!C) { FPlatformMisc::RequestExitWithStatus(false,1);return; }
    PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::LeftMouseButton,IE_Released,0.f));
    PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::LeftMouseButton,IE_DoubleClick,1.f));
    ++SmokeVolleyClicks;
    FTimerHandle T;GetWorldTimerManager().SetTimer(T,this,SmokeVolleyClicks<4?&ASpychoGameMode::SmokeAimVolley:&ASpychoGameMode::SmokeCheckVolley,.17f,false);
}
void ASpychoGameMode::SmokeCheckVolley()
{
    auto* PC=GetWorld()->GetFirstPlayerController();auto* C=PC?Cast<ASpychoCharacter>(PC->GetPawn()):nullptr;
    if (!C) { FPlatformMisc::RequestExitWithStatus(false,1);return; }
    bool Volley=C->Handgun->Magazine==0&&C->GetShotFeedbackCount()==SmokeFeedbackBefore+6&&C->GetAimAlpha()>.98f;
    UE_LOG(LogTemp,Display,TEXT("SPYCHO_SMOKE %s all six rapid ADS clicks accepted exactly once"),Volley?TEXT("PASS"):TEXT("FAIL"));bSmokeAmmoPassed &= Volley;
    PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::LeftMouseButton,IE_Released,0.f));
    PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::RightMouseButton,IE_Released,0.f));
    PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::R,IE_Pressed,1.f));
    FTimerHandle T;GetWorldTimerManager().SetTimer(T,this,&ASpychoGameMode::RunSmokeTest,2.5f,false);
}
void ASpychoGameMode::RunSmokeTest()
{
    // Runs against the saved House map and real collision, damage and round systems.
    auto* PC=GetWorld()->GetFirstPlayerController(); auto* Shooter=PC?Cast<ASpychoCharacter>(PC->GetPawn()):nullptr;
    ASpychoCharacter* Bot=nullptr; for (TActorIterator<ASpychoCharacter> It(GetWorld()); It; ++It) if (It->bTestOpponent) Bot=*It;
    if (!Shooter || !Bot) { UE_LOG(LogTemp,Error,TEXT("SPYCHO_SMOKE FAIL missing pawns")); FPlatformMisc::RequestExitWithStatus(false,1); return; }
    bSmokeAmmoPassed &= Shooter->Handgun->Magazine==6 && Shooter->Handgun->Reserve==6 && !Shooter->Handgun->bReloading;
    UE_LOG(LogTemp,Display,TEXT("SPYCHO_SMOKE %s rate limit and timed reload"),bSmokeAmmoPassed?TEXT("PASS"):TEXT("FAIL"));
    Bot->SetActorLocation(FVector(350,260,90));
    Shooter->SetActorLocation(FVector(350,0,90));
    FVector Origin=Shooter->Camera->GetComponentLocation(); // through y=85 paper partition away from the x=470 doorway
    FRotator Aim=(Bot->GetActorLocation()-Origin).Rotation(); Shooter->Handgun->Fire(Aim);
    auto* GS=GetGameState<ASpychoGameState>();
    bool Pass=Bot->Health->Health<=0.f && Shooter->Handgun->Magazine==5 && !GS->bRoundActive && GS->Evidence.Num()>=2;
    UE_LOG(LogTemp,Display,TEXT("SPYCHO_SMOKE %s wall kill: HP=%.1f ammo=%d marks=%d roundActive=%d"),Pass?TEXT("PASS"):TEXT("FAIL"),Bot->Health->Health,Shooter->Handgun->Magazine,GS->Evidence.Num(),GS->bRoundActive);
    // A masonry exterior wall must stop exactly the same shot.
    GS->bRoundActive=true; Bot->Health->Health=100.f; Bot->SetActorLocation(FVector(800,260,90));
    Shooter->SetActorLocation(FVector(600,260,90)); Origin=Shooter->Camera->GetComponentLocation();
    Shooter->Penetration->Fire(Origin,(Bot->GetActorLocation()-Origin).GetSafeNormal());
    bool Blocked=Bot->Health->Health==100.f;
    UE_LOG(LogTemp,Display,TEXT("SPYCHO_SMOKE %s masonry stops bullet HP=%.1f"),Blocked?TEXT("PASS"):TEXT("FAIL"),Bot->Health->Health);
    ResetRound(); bool Cleared=GS->Evidence.IsEmpty();
    UE_LOG(LogTemp,Display,TEXT("SPYCHO_SMOKE %s round reset clears evidence"),Cleared?TEXT("PASS"):TEXT("FAIL"));
    bool AudioReady=GS->Sounds.Num()==9&&GS->FootstepVariants.Num()==9;
    for (const auto& S : GS->Sounds) AudioReady &= S!=nullptr;
    for (const auto& S : GS->FootstepVariants) AudioReady &= S!=nullptr;
    UE_LOG(LogTemp,Display,TEXT("SPYCHO_SMOKE %s all 18 recorded sound references exist"),AudioReady?TEXT("PASS"):TEXT("FAIL"));
    bool DoorReady=false; for (TActorIterator<ASpychoDoor> It(GetWorld()); It; ++It) { It->Toggle(true); DoorReady=It->bOpen && It->bCareful; It->Reset(); break; }
    UE_LOG(LogTemp,Display,TEXT("SPYCHO_SMOKE %s door careful toggle/reset"),DoorReady?TEXT("PASS"):TEXT("FAIL"));
    bSmokeWorldPassed=Pass&&Blocked&&Cleared&&bSmokeAmmoPassed&&bSmokeMovementPassed&&AudioReady&&DoorReady;
    PC->GetPawn()->SetActorLocation(FVector(-335,34,90));PC->SetControlRotation(FRotator(0,0,0));
    // Use the actual E input while looking slightly above the panel center,
    // and during the round-end countdown shown in the user's screenshot.
    GS->bRoundActive=false;
    PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::E,IE_Pressed,1.f));
    FTimerHandle T;GetWorldTimerManager().SetTimer(T,this,&ASpychoGameMode::SmokeCheckDoorInput,.15f,false);
}
void ASpychoGameMode::SmokeCheckDoorInput()
{
    bool Open=false,Sealed=true;auto* PC=GetWorld()->GetFirstPlayerController();
    for (TActorIterator<ASpychoDoor> It(GetWorld());It;++It) if (It->GetActorLocation().X<-250.f) Open|=It->bOpen;
    for (TActorIterator<ASpychoDoor> It(GetWorld());It;++It)
    {
        It->Reset();
        for (float Y : {1.f,5.f,50.f,96.f,100.f}) for (float Z : {20.f,154.f,208.f})
        {
            FVector A=It->GetActorTransform().TransformPosition(FVector(-70,Y,Z));
            FVector B=It->GetActorTransform().TransformPosition(FVector(70,Y,Z));
            FHitResult H;FCollisionQueryParams Q(SCENE_QUERY_STAT(SpychoDoorSeal),false,PC->GetPawn());
            Sealed &= GetWorld()->LineTraceSingleByChannel(H,A,B,ECC_Visibility,Q);
        }
    }
    UE_LOG(LogTemp,Display,TEXT("SPYCHO_SMOKE %s actual E opens nearby door during countdown"),Open?TEXT("PASS"):TEXT("FAIL"));
    UE_LOG(LogTemp,Display,TEXT("SPYCHO_SMOKE %s closed doors block sight at both edges and header"),Sealed?TEXT("PASS"):TEXT("FAIL"));
    FPlatformMisc::RequestExitWithStatus(false,bSmokeWorldPassed&&Open&&Sealed?0:1);
}
void ASpychoGameMode::BeginBotSmoke()
{
    auto* PC=GetWorld()->GetFirstPlayerController();
    if (!PC || !PC->GetPawn()) { FPlatformMisc::RequestExitWithStatus(false,1);return; }
    PC->GetPawn()->SetActorLocation(FVector(350,-260,90));
    for (TActorIterator<ASpychoHunterController> It(GetWorld());It;++It) It->SetActorTickEnabled(true);
    FTimerHandle T;GetWorldTimerManager().SetTimer(T,this,&ASpychoGameMode::CheckBotSilence,3.f,false);
}
void ASpychoGameMode::BeginPolishSmoke()
{
    StartMatch();auto* PC=GetWorld()->GetFirstPlayerController();auto* C=Cast<ASpychoCharacter>(PC->GetPawn());
    bool Assets=C->FireAnimation&&C->ReloadAnimation;
    UE_LOG(LogTemp,Display,TEXT("SPYCHO_POLISH %s fire/reload animation references"),Assets?TEXT("PASS"):TEXT("FAIL"));bPolishPassed&=Assets;
    C->SetActorLocation(FVector(-335,34,90));PC->SetControlRotation(FRotator::ZeroRotator);
    auto* NearDoor=C->GetUsableDoor();bool DoorSafety=NearDoor!=nullptr;
    if (NearDoor)
    {
        NearDoor->Toggle(false,C);DoorSafety&=NearDoor->bOpen&&NearDoor->SwingAngle<0;
        C->SetActorLocation(NearDoor->GetActorTransform().TransformPosition(FVector(0,50,90)));
        NearDoor->Toggle(false,C);DoorSafety&=NearDoor->bOpen;
        C->SetActorLocation(FVector(-335,34,90));NearDoor->Toggle(false,C);DoorSafety&=!NearDoor->bOpen;NearDoor->Reset();
    }
    UE_LOG(LogTemp,Display,TEXT("SPYCHO_POLISH %s door opens away, guards occupied frame, closes beside operator"),DoorSafety?TEXT("PASS"):TEXT("FAIL"));bPolishPassed&=DoorSafety;
    for (TActorIterator<ASpychoHunterController> It(GetWorld());It;++It)
    {
        bool Routes=It->ValidateRouteClearance();bPolishPassed&=Routes;
        UE_LOG(LogTemp,Display,TEXT("SPYCHO_POLISH %s all room routes have capsule clearance"),Routes?TEXT("PASS"):TEXT("FAIL"));
    }
    FVector Source(470,260,154),Listener(470,0,154);
    auto Closed=SpychoAcoustics::Probe(GetWorld(),Source,Listener,C);
    for (TActorIterator<ASpychoDoor> It(GetWorld());It;++It)
        if (FVector::Dist2D(It->GetActorLocation(),FVector(520.5f,85,0))<5.f)
        { It->Toggle(true);It->Swing->SetRelativeRotation(FRotator(0,It->SwingAngle,0)); }
    auto Open=SpychoAcoustics::Probe(GetWorld(),Source,Listener,C);
    bool Acoustics=Open.Transmission()>Closed.Transmission()&&Open.Cutoff()>Closed.Cutoff()&&Closed.bClosedDoor;
    UE_LOG(LogTemp,Display,TEXT("SPYCHO_POLISH %s closed/open door acoustic contrast %.2f/%.2f"),Acoustics?TEXT("PASS"):TEXT("FAIL"),Closed.Transmission(),Open.Transmission());bPolishPassed&=Acoustics;
    for (TActorIterator<ASpychoDoor> It(GetWorld());It;++It) It->Reset();
    C->SetActorLocation(FVector(-200,230,90));PC->SetControlRotation(FRotator(0,0,0));
    for (TActorIterator<ASpychoHunterController> It(GetWorld());It;++It)
    { It->GetPawn()->SetActorLocation(FVector(470,270,90));It->SetActorTickEnabled(false); }
    PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::Q,IE_Pressed,1.f));
    FTimerHandle T;GetWorldTimerManager().SetTimer(T,this,&ASpychoGameMode::CheckPolishCoin,3.f,false);
}
void ASpychoGameMode::CheckPolishCoin()
{
    auto* PC=GetWorld()->GetFirstPlayerController();auto* C=Cast<ASpychoCharacter>(PC->GetPawn());
    ASpychoDistraction* Coin=nullptr;for (TActorIterator<ASpychoDistraction> It(GetWorld());It;++It) Coin=*It;
    ASpychoHunterController* Hunter=nullptr;for (TActorIterator<ASpychoHunterController> It(GetWorld());It;++It) Hunter=*It;
    bool Landing=Coin&&Coin->AudibleBounces>0&&C->Coins==1&&Hunter&&Hunter->HeardEvents>0&&!Hunter->bWallShotAllowed
        &&FVector::Dist2D(Hunter->LastKnown,Coin->FirstLanding)<100.f&&FVector::Dist2D(Hunter->LastKnown,C->GetActorLocation())>150.f;
    UE_LOG(LogTemp,Display,TEXT("SPYCHO_POLISH %s Q coin bounces; bot hears landing, not thrower (coins %d bounces %d heard %d)"),Landing?TEXT("PASS"):TEXT("FAIL"),C->Coins,Coin?Coin->AudibleBounces:0,Hunter?Hunter->HeardEvents:0);bPolishPassed&=Landing;
    if (Hunter) { PolishEstimate=Hunter->LastKnown;Hunter->MemoryUntil=0;Hunter->PauseUntil=0;Hunter->InvestigateAt=0;Hunter->SetActorTickEnabled(true); }
    C->SetActorLocation(FVector(-600,-300,90));
    FTimerHandle T;GetWorldTimerManager().SetTimer(T,this,&ASpychoGameMode::CheckPolishSearch,1.f,false);
}
void ASpychoGameMode::CheckPolishSearch()
{
    bool Search=false;for (TActorIterator<ASpychoHunterController> It(GetWorld());It;++It)
    {
        Search=It->State==ESpychoHunterState::Investigate&&It->LastKnown.Equals(PolishEstimate,1.f)&&!It->bWallShotAllowed&&It->ShotsTaken==0;
        It->SetActorTickEnabled(false);
    }
    UE_LOG(LogTemp,Display,TEXT("SPYCHO_POLISH %s uncertain investigation persists without tracking hidden player"),Search?TEXT("PASS"):TEXT("FAIL"));bPolishPassed&=Search;
    auto* PC=GetWorld()->GetFirstPlayerController();PC->GetPawn()->SetActorLocation(FVector(-335,34,90));PC->SetControlRotation(FRotator::ZeroRotator);
    FTimerHandle T;GetWorldTimerManager().SetTimer(T,this,&ASpychoGameMode::CheckPolishMatch,.6f,false);
}
void ASpychoGameMode::CheckPolishMatch()
{
    auto* PC=GetWorld()->GetFirstPlayerController();auto* C=Cast<ASpychoCharacter>(PC->GetPawn());
    bool Lowered=C->GetWallLowering()>.6f;
    UE_LOG(LogTemp,Display,TEXT("SPYCHO_POLISH %s close wall lowers weapon %.2f"),Lowered?TEXT("PASS"):TEXT("FAIL"),C->GetWallLowering());bPolishPassed&=Lowered;
    auto* GS=GetGameState<ASpychoGameState>();GS->ScoreA=0;GS->ScoreB=1;ResetRound();
    C=Cast<ASpychoCharacter>(PC->GetPawn());bool Swapped=C->GetActorLocation().X>300&&C->Coins==2&&GS->ScoreB==1;
    GS->ScoreA=2;GS->ScoreB=2;
    for (TActorIterator<ASpychoCharacter> It(GetWorld());It;++It) if (It->bTestOpponent) { It->Health->Damage(1000.f);break; }
    bool Won=GS->ScoreA==3&&GS->ScoreB==2&&GS->bMatchOver&&!GS->bRoundActive&&!GetWorldTimerManager().IsTimerActive(ResetTimer);
    bPolishPassed&=Swapped&&Won;
    UE_LOG(LogTemp,Display,TEXT("SPYCHO_POLISH %s alternating starts and decisive fifth round (swap %d won %d)"),Swapped&&Won?TEXT("PASS"):TEXT("FAIL"),Swapped,Won);
    PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::Enter,IE_Pressed,1.f));
    FTimerHandle T;GetWorldTimerManager().SetTimer(T,this,&ASpychoGameMode::CheckPolishRematch,.15f,false);
}
void ASpychoGameMode::CheckPolishRematch()
{
    auto* GS=GetGameState<ASpychoGameState>();auto* PC=GetWorld()->GetFirstPlayerController();
    bool Rematch=GS->ScoreA==0&&GS->ScoreB==0&&!GS->bMatchOver&&GS->bRoundActive&&Cast<ASpychoCharacter>(PC->GetPawn())->Coins==2;
    UE_LOG(LogTemp,Display,TEXT("SPYCHO_POLISH %s Enter rematch resets match and replenishes coins"),Rematch?TEXT("PASS"):TEXT("FAIL"));bPolishPassed&=Rematch;
    FPlatformMisc::RequestExitWithStatus(false,bPolishPassed?0:1);
}
void ASpychoGameMode::CheckBotSilence()
{
    bool Silent=true;
    for (TActorIterator<ASpychoHunterController> It(GetWorld());It;++It)
    {
        Silent &= It->HeardEvents==0&&It->ShotsTaken==0&&It->MemoryUntil==0&&It->LastKnown.IsNearlyZero();
        It->PlanRoute(9);It->PauseUntil=0;
    }
    UE_LOG(LogTemp,Display,TEXT("SPYCHO_BOT %s stationary hidden player gives no room knowledge or shots"),Silent?TEXT("PASS"):TEXT("FAIL"));bBotSmokePassed &= Silent;
    GetWorld()->GetFirstPlayerController()->GetPawn()->SetActorLocation(FVector(-950,-300,90));
    FTimerHandle T;GetWorldTimerManager().SetTimer(T,this,&ASpychoGameMode::CheckBotRoam,12.f,false);
}
void ASpychoGameMode::CheckBotRoam()
{
    auto* PC=GetWorld()->GetFirstPlayerController(); auto* C=PC?Cast<ASpychoCharacter>(PC->GetPawn()):nullptr;
    ASpychoHunterController* Hunter=nullptr;for (TActorIterator<ASpychoHunterController> It(GetWorld());It;++It) Hunter=*It;
    auto* Bot=Hunter?Cast<ASpychoCharacter>(Hunter->GetPawn()):nullptr;
    if (!C || !Bot) { FPlatformMisc::RequestExitWithStatus(false,1);return; }
    bool Open=false;for (TActorIterator<ASpychoDoor> It(GetWorld());It;++It) Open|=It->bOpen;
    bool Roam=Bot->GetFootstepCount()>0&&Open&&Bot->GetActorLocation().Y<85.f;
    UE_LOG(LogTemp,Display,TEXT("SPYCHO_BOT %s autonomous doorway traversal and real footsteps (%d, y=%.1f)"),Roam?TEXT("PASS"):TEXT("FAIL"),Bot->GetFootstepCount(),Bot->GetActorLocation().Y);bBotSmokePassed &= Roam;
    C->SetActorLocation(FVector(350,0,90));Bot->SetActorLocation(FVector(350,260,90));Bot->GetCharacterMovement()->StopMovementImmediately();
    int32 Before=Hunter->HeardEvents;
    Hunter->HearNoise(ESpychoNoise::Wood,C->GetActorLocation(),.005f,C);
    bool Quiet=Hunter->HeardEvents==Before;
    Hunter->HearNoise(ESpychoNoise::Gunshot,C->GetActorLocation(),2.8f,C);
    bool Heard=Hunter->HeardEvents==Before+1;
    UE_LOG(LogTemp,Display,TEXT("SPYCHO_BOT %s distant quiet step ignored, gunshot heard"),Quiet&&Heard?TEXT("PASS"):TEXT("FAIL"));bBotSmokePassed &= Quiet&&Heard;
    FTimerHandle T;GetWorldTimerManager().SetTimer(T,this,&ASpychoGameMode::CheckBotWallShot,3.5f,false);
}
void ASpychoGameMode::CheckBotWallShot()
{
    auto* GS=GetGameState<ASpychoGameState>();
    ASpychoHunterController* Hunter=nullptr;for (TActorIterator<ASpychoHunterController> It(GetWorld());It;++It) Hunter=*It;
    bool Shot=Hunter&&Hunter->ShotsTaken>0&&GS&&GS->Evidence.Num()>=2;
    UE_LOG(LogTemp,Display,TEXT("SPYCHO_BOT %s inferred shot through opaque paper partition"),Shot?TEXT("PASS"):TEXT("FAIL"));bBotSmokePassed &= Shot;
    ResetRound();auto* PC=GetWorld()->GetFirstPlayerController();if (!PC || !PC->GetPawn()) { FPlatformMisc::RequestExitWithStatus(false,1);return; }
    PC->GetPawn()->SetActorLocation(FVector(350,330,90));
    for (TActorIterator<ASpychoHunterController> It(GetWorld());It;++It)
    {
        It->GetPawn()->SetActorLocation(FVector(350,180,90));It->SetControlRotation(FRotator(0,90,0));It->GetPawn()->SetActorRotation(FRotator(0,90,0));It->SetActorTickEnabled(true);
    }
    FTimerHandle T;GetWorldTimerManager().SetTimer(T,this,&ASpychoGameMode::CheckBotDuel,2.f,false);
}
void ASpychoGameMode::CheckBotDuel()
{
    auto* C=Cast<ASpychoCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn());auto* GS=GetGameState<ASpychoGameState>();
    bool Lethal=C&&C->Health->Health<=0&&GS&&!GS->bRoundActive;
    UE_LOG(LogTemp,Display,TEXT("SPYCHO_BOT %s visible hunter can kill player and end round"),Lethal?TEXT("PASS"):TEXT("FAIL"));bBotSmokePassed &= Lethal;
    FPlatformMisc::RequestExitWithStatus(false,bBotSmokePassed?0:1);
}
