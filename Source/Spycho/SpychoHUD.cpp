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
    auto* C=PC?Cast<ASpychoCharacter>(PC->GetPawn()):nullptr;
    auto* GS=GetWorld()->GetGameState<ASpychoGameState>();
    FLinearColor Muted(0.55f,0.55f,0.5f);
    if (C) DrawText(C->Handgun->bReloading?TEXT("reloading"):C->Handgun->Magazine==0?TEXT("Empty — R reload"):FString::Printf(TEXT("%d / %d"),C->Handgun->Magazine,C->Handgun->Reserve),Muted,Canvas->SizeX-160,Canvas->SizeY-55,nullptr,1.1f);
    if (GS&&C)
    {
        int32 You=C->DuelSlot==0?GS->ScoreA:GS->ScoreB,Other=C->DuelSlot==0?GS->ScoreB:GS->ScoreA;
        DrawRect(FLinearColor(.01f,.01f,.01f,.4f),25,25,230,22);
        DrawText(FString::Printf(TEXT("YOU %d   /   %d OPPONENT     FIRST TO 3"),You,Other),Muted,30,30,nullptr,.85f);
        if (!GS->bRoundActive)
        {
            FString Message=GS->bMatchOver?(You>Other?TEXT("You won the match."):TEXT("You lost the match.")):TEXT("Round over. Switching positions in 5 seconds.");
            DrawText(Message,FLinearColor(.8f,.78f,.7f),Canvas->SizeX*.32f,Canvas->SizeY*.44f,nullptr,1.3f);
            if (GS->bMatchOver) DrawText(PC->HasAuthority()?TEXT("Enter — rematch"):TEXT("Waiting for host to rematch"),Muted,Canvas->SizeX*.36f,Canvas->SizeY*.5f);
        }
        if (C->Health->Health>0) if (auto* Door=C->GetUsableDoor())
            DrawText(Door->bOpen?TEXT("E  Close door"):TEXT("E  Open door   |   Ctrl + E  quietly"),Muted,Canvas->SizeX*.38f,Canvas->SizeY*.7f,nullptr,.9f);
        DrawText(FString::Printf(TEXT("Q  coin  %d"),C->Coins),Muted,Canvas->SizeX-160,Canvas->SizeY-85,nullptr,.85f);
    }
    if (GetWorld()->GetTimeSeconds()<18.f) DrawText(TEXT("WASD  move   Shift  sprint   Ctrl  slow walk   E  door   R  reload   Q  toss coin"),Muted,30,Canvas->SizeY-55,nullptr,.85f);
    if (PC && PC->bDebug)
    {
        DrawText(TEXT("DEVELOPER  F3 hide  |  F5 host reset"),FLinearColor::Yellow,20,20);
        if (GS) DrawText(FString::Printf(TEXT("round %d  %s\n%s"),GS->Round,*GS->RoundMessage,*GS->LastShotDebug),FLinearColor::Yellow,20,45);
        if (C) DrawText(FString::Printf(TEXT("health %.1f  role %d  %s"),C->Health->Health,int32(C->GetLocalRole()),C->bIsCrouched?TEXT("crouched"):(C->bCareful?TEXT("careful"):TEXT("walking"))),FLinearColor::Yellow,20,100);
        for (TActorIterator<ASpychoCharacter> It(GetWorld()); It; ++It) if (*It!=C) DrawDebugString(GetWorld(),It->GetActorLocation(),FString::Printf(TEXT("HP %.0f"),It->Health->Health),nullptr,FColor::Yellow,0.f);
        for (TActorIterator<ASpychoHunterController> It(GetWorld());It;++It) DrawText(FString::Printf(TEXT("bot: %s"),It->StateName()),FLinearColor::Yellow,20,130);
    }
}
