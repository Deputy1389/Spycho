#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "SpychoGameState.generated.h"
class USoundBase;
class USoundAttenuation;
class UReverbEffect;
class UAudioComponent;
struct FSpychoVoice
{
    TWeakObjectPtr<UAudioComponent> Audio;
    FVector Source;
    float Gain=1;
    float Volume=1;
    float Cutoff=18000;
    bool Ambient=false;
};

UENUM()
enum class ESpychoNoise : uint8 { Wood, Carpet, Tile, Gunshot, Reload, Door, Impact, Creak, Distraction };

UCLASS()
class SPYCHO_API ASpychoGameState : public AGameStateBase
{
    GENERATED_BODY()
public:
    ASpychoGameState();
    UPROPERTY(ReplicatedUsing=OnRep_Round) int32 Round = 0;
    UPROPERTY(Replicated) bool bRoundActive = true;
    UPROPERTY(Replicated) FString RoundMessage = TEXT("Listen carefully.");
    UPROPERTY(Replicated) int32 ScoreA=0;
    UPROPERTY(Replicated) int32 ScoreB=0;
    UPROPERTY(Replicated) bool bMatchOver=false;
    UPROPERTY(Replicated) int32 BotDifficulty=1;
    UPROPERTY(Replicated) double RoundStartsAt=0;
    UPROPERTY(Replicated) double RoundEndsAt=0;
    UPROPERTY(Replicated) double ResultAt=0;
    UPROPERTY(Replicated) int32 RoundWinner=-1;
    UPROPERTY(Replicated) FString ResultReason;
    bool CanShoot() const { return bRoundActive && GetServerWorldTimeSeconds()>=RoundStartsAt; }
    UPROPERTY() TArray<TObjectPtr<USoundBase>> Sounds;
    UPROPERTY() TArray<TObjectPtr<USoundBase>> FootstepVariants;
    UPROPERTY() TObjectPtr<USoundAttenuation> Attenuation;
    UPROPERTY() TObjectPtr<UReverbEffect> HardRoomReverb;
    UPROPERTY() TObjectPtr<UReverbEffect> SoftRoomReverb;
    virtual void Tick(float DeltaSeconds) override;
    UPROPERTY() TArray<TObjectPtr<AActor>> Evidence;
    FString LastShotDebug;
    int32 ListenerRoom=-2;
    TArray<FSpychoVoice> Voices;
    UFUNCTION(NetMulticast, Unreliable) void Noise(ESpychoNoise Kind, FVector Location, float Gain);
    UFUNCTION(NetMulticast, Reliable) void Impact(FVector Location, FVector Normal, bool bExit);
    UFUNCTION(NetMulticast, Reliable) void ShotDebug(FVector Start, FVector End, const FString& Detail);
    UFUNCTION() void OnRep_Round();
    bool IsDebug() const;
    void NotifyHearing(ESpychoNoise Kind,FVector Location,float Gain,AActor* Source);
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& Out) const override;
};
