#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SpychoHandgun.generated.h"
class USpychoPenetration;
UCLASS(ClassGroup=(Spycho))
class SPYCHO_API USpychoHandgun : public UActorComponent
{
    GENERATED_BODY()
public:
    USpychoHandgun();
    UPROPERTY(Replicated, BlueprintReadOnly) int32 Magazine = 6;
    UPROPERTY(Replicated, BlueprintReadOnly) int32 Reserve = 12;
    UPROPERTY(Replicated, BlueprintReadOnly) bool bReloading = false;
    UPROPERTY(EditAnywhere) float ReloadSeconds = 2.1f;
    UPROPERTY(EditAnywhere) float ShotInterval = 0.16f;
    void Fire(FRotator Aim, int32 PredictionKey = 0);
    void Reload();
    void CancelReload();
    UFUNCTION(NetMulticast, Reliable) void Fired(int32 PredictionKey);
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& Out) const override;
private:
    double NextShot = 0;
    FTimerHandle ReloadTimer;
    FTimerHandle BufferedShotTimer;
    bool bBufferedShot = false;
    void FlushBufferedShot(FRotator Aim, int32 PredictionKey);
    void FinishReload();
};
