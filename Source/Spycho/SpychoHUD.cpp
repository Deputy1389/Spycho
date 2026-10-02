#include "SpychoHUD.h"
#include "SpychoCharacter.h"
#include "SpychoHealth.h"
#include "SpychoHandgun.h"
#include "SpychoGameState.h"
#include "SpychoPlayerController.h"
#include "Engine/Canvas.h"
#include "EngineUtils.h"
#include "DrawDebugHelpers.h"
void ASpychoHUD::DrawHUD()
{
    Super::DrawHUD(); if (!Canvas) return;
    auto* PC=Cast<ASpychoPlayerController>(GetOwningPlayerController());
    auto* C=PC?Cast<ASpychoCharacter>(PC->GetPawn()):nullptr;
    auto* GS=GetWorld()->GetGameState<ASpychoGameState>();
    FLinearColor Muted(0.55f,0.55f,0.5f);
    if (C) DrawText(C->Handgun->bReloading?TEXT("reloading"):C->Handgun->Magazine==0?TEXT("Empty — R reload"):FString::Printf(TEXT("%d / %d"),C->Handgun->Magazine,C->Handgun->Reserve),Muted,Canvas->SizeX-160,Canvas->SizeY-55,nullptr,1.1f);
    if (GS && !GS->bRoundActive) DrawText(GS->RoundMessage,Muted,Canvas->SizeX*0.4f,Canvas->SizeY*0.46f,nullptr,1.3f);
    if (GetWorld()->GetTimeSeconds()<18.f) DrawText(TEXT("WASD  move   Shift  sprint   Ctrl  slow walk   E  door   R  reload"),Muted,30,Canvas->SizeY-55);
    if (PC && PC->bDebug)
    {
        DrawText(TEXT("DEVELOPER  F3 hide  |  F5 host reset"),FLinearColor::Yellow,20,20);
        if (GS) DrawText(FString::Printf(TEXT("round %d  %s\n%s"),GS->Round,*GS->RoundMessage,*GS->LastShotDebug),FLinearColor::Yellow,20,45);
        if (C) DrawText(FString::Printf(TEXT("health %.1f  role %d  %s"),C->Health->Health,int32(C->GetLocalRole()),C->bIsCrouched?TEXT("crouched"):(C->bCareful?TEXT("careful"):TEXT("walking"))),FLinearColor::Yellow,20,100);
        for (TActorIterator<ASpychoCharacter> It(GetWorld()); It; ++It) if (*It!=C) DrawDebugString(GetWorld(),It->GetActorLocation(),FString::Printf(TEXT("HP %.0f"),It->Health->Health),nullptr,FColor::Yellow,0.f);
    }
}
