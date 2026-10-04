#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SpychoHealth.generated.h"

UCLASS(ClassGroup=(Spycho), meta=(BlueprintSpawnableComponent))
class SPYCHO_API USpychoHealth : public UActorComponent
{
    GENERATED_BODY()
public:
    USpychoHealth();
    UPROPERTY(ReplicatedUsing=OnRep_Health, BlueprintReadOnly) float Health = 100.f;
    void Damage(float Amount);
    UFUNCTION() void OnRep_Health();
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& Out) const override;
};
