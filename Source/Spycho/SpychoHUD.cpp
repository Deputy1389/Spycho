#include "SpychoHUD.h"
#include "SpychoCharacter.h"
#include "SpychoHealth.h"
#include "SpychoHandgun.h"
#include "SpychoGameState.h"
#include "SpychoPlayerController.h"
#include "Engine/Canvas.h"
#include "EngineUtils.h"
#include "DrawDebugHelpers.h"
#include "SpychoDoor.h"
#include "SpychoHunterController.h"
void ASpychoHUD::DrawHUD()
{
    Super::DrawHUD(); if (!Canvas) return;
    auto* PC=Cast<ASpychoPlayerController>(GetOwningPlayerController());
    if (PC&&PC->bMenuOpen) return;
    auto* C=PC?Cast<ASpychoCharacter>(PC->GetPawn()):nullptr;
    auto* GS=GetWorld()->GetGameState<ASpychoGameState>();
    FLinearColor Muted(0.78f,0.79f,0.71f);
    if (C) DrawText(C->Handgun->bReloading?TEXT("reloading"):C->Handgun->Magazine==0?TEXT("Empty — R reload"):FString::Printf(TEXT("%d / %d"),C->Handgun->Magazine,C->Handgun->Reserve),Muted,Canvas->SizeX-160,Canvas->SizeY-55,nullptr,1.1f);
    if (GS&&C)
    {
        int32 You=C->DuelSlot==0?GS->ScoreA:GS->ScoreB,Other=C->DuelSlot==0?GS->ScoreB:GS->ScoreA;
        DrawRect(FLinearColor(.01f,.01f,.01f,.4f),25,25,230,22);
        DrawText(FString::Printf(TEXT("YOU %d   /   %d OPPONENT     FIRST TO 3"),You,Other),Muted,30,30,nullptr,.85f);
        float Now=GS->GetServerWorldTimeSeconds();
        if (GS->bRoundActive)
        {
            int32 Seconds=FMath::Max(0,FMath::CeilToInt(GS->RoundEndsAt-Now));
            DrawRect(FLinearColor(.01f,.015f,.012f,.55f),Canvas->SizeX*.5f-35,25,70,28);
            DrawText(FString::Printf(TEXT("%d:%02d"),Seconds/60,Seconds%60),Seconds<=15?FLinearColor(.95f,.68f,.42f):Muted,Canvas->SizeX*.5f-20,30,nullptr,1.1f);
            if (Now<GS->RoundStartsAt)
            {
                DrawRect(FLinearColor(.01f,.015f,.012f,.75f),Canvas->SizeX*.5f-160,Canvas->SizeY*.38f,320,95);
                DrawText(FString::Printf(TEXT("GET READY   %d"),FMath::CeilToInt(GS->RoundStartsAt-Now)),FLinearColor(.92f,.84f,.62f),Canvas->SizeX*.5f-108,Canvas->SizeY*.4f,nullptr,1.6f);
                DrawText(TEXT("Listen before you move."),Muted,Canvas->SizeX*.5f-95,Canvas->SizeY*.4f+36);
            }
        }
        if (!GS->bRoundActive)
        {
            bool Draw=GS->RoundWinner==-2;bool Won=GS->RoundWinner==C->DuelSlot;
            FString Message=GS->bMatchOver?(You==Other?TEXT("MATCH DRAWN"):You>Other?TEXT("MATCH WON"):TEXT("MATCH LOST")):(Draw?TEXT("TIME EXPIRED"):Won?TEXT("ROUND WON"):TEXT("ROUND LOST"));
            float X=Canvas->SizeX*.5f-280,Y=Canvas->SizeY*.38f;
            DrawRect(FLinearColor(.008f,.012f,.011f,.85f),X,Y,560,150);
            DrawText(Message,FLinearColor(.92f,.84f,.62f),X+22,Y+18,nullptr,1.6f);
            if (!Won||Draw)
            {
                FString Reason=GS->ResultReason;int32 BreakAt=Reason.Len()>80?Reason.Left(80).Find(TEXT(" "),ESearchCase::CaseSensitive,ESearchDir::FromEnd):INDEX_NONE;
                DrawText(BreakAt>0?Reason.Left(BreakAt):Reason,Muted,X+22,Y+58,nullptr,.85f);
                if (BreakAt>0) DrawText(Reason.Mid(BreakAt+1),Muted,X+22,Y+78,nullptr,.85f);
            }
            FString Next=GS->bMatchOver?(PC->HasAuthority()?TEXT("Enter — rematch   |   Escape — menu"):TEXT("Waiting for host to rematch   |   Escape — menu")):FString::Printf(TEXT("Switching positions in %d"),FMath::Max(0,FMath::CeilToInt(5.f-(Now-GS->ResultAt))));
            DrawText(Next,Muted,X+22,Y+110,nullptr,.95f);
        }
        if (C->Health->Health>0) if (auto* Door=C->GetUsableDoor())
            DrawText(Door->bOpen?TEXT("E  Close door"):TEXT("E  Open door   |   Ctrl + E  quietly"),Muted,Canvas->SizeX*.38f,Canvas->SizeY*.7f,nullptr,.9f);
        DrawText(FString::Printf(TEXT("Q  coin  %d"),C->Coins),Muted,Canvas->SizeX-160,Canvas->SizeY-85,nullptr,.85f);
    }
    if (GetWorld()->GetTimeSeconds()<18.f) DrawText(TEXT("WASD move   Shift sprint   Ctrl slow walk   E door   R reload   Q coin   Esc menu"),Muted,30,Canvas->SizeY-55,nullptr,.85f);
    if (PC && PC->bDebug)
    {
        DrawText(TEXT("DEVELOPER  F3 hide  |  F5 host reset"),FLinearColor::Yellow,20,20);
        if (GS) DrawText(FString::Printf(TEXT("round %d  %s\n%s"),GS->Round,*GS->RoundMessage,*GS->LastShotDebug),FLinearColor::Yellow,20,45);
        if (C) DrawText(FString::Printf(TEXT("health %.1f  role %d  %s"),C->Health->Health,int32(C->GetLocalRole()),C->bIsCrouched?TEXT("crouched"):(C->bCareful?TEXT("careful"):TEXT("walking"))),FLinearColor::Yellow,20,100);
        for (TActorIterator<ASpychoCharacter> It(GetWorld()); It; ++It) if (*It!=C) DrawDebugString(GetWorld(),It->GetActorLocation(),FString::Printf(TEXT("HP %.0f"),It->Health->Health),nullptr,FColor::Yellow,0.f);
        for (TActorIterator<ASpychoHunterController> It(GetWorld());It;++It) DrawText(FString::Printf(TEXT("bot: %s"),It->StateName()),FLinearColor::Yellow,20,130);
    }
}
