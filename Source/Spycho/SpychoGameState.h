#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "SpychoGameState.generated.h"
class USoundBase;
class USoundAttenuation;

UENUM()
enum class ESpychoNoise : uint8 { Wood, Carpet, Tile, Gunshot, Reload, Door, Impact, Creak };

UCLASS()
class SPYCHO_API ASpychoGameState : public AGameStateBase
{
    GENERATED_BODY()
public:
    ASpychoGameState();
    UPROPERTY(ReplicatedUsing=OnRep_Round) int32 Round = 0;
    UPROPERTY(Replicated) bool bRoundActive = true;
    UPROPERTY(Replicated) FString RoundMessage = TEXT("Listen carefully.");
    UPROPERTY() TArray<TObjectPtr<USoundBase>> Sounds;
    UPROPERTY() TObjectPtr<USoundAttenuation> Attenuation;
    UPROPERTY() TArray<TObjectPtr<AActor>> Evidence;
    FString LastShotDebug;
    UFUNCTION(NetMulticast, Unreliable) void Noise(ESpychoNoise Kind, FVector Location, float Gain);
    UFUNCTION(NetMulticast, Reliable) void Impact(FVector Location, FVector Normal, bool bExit);
    UFUNCTION(NetMulticast, Reliable) void ShotDebug(FVector Start, FVector End, const FString& Detail);
    UFUNCTION() void OnRep_Round();
    bool IsDebug() const;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& Out) const override;
};
