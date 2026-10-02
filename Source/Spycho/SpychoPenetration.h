#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SpychoPenetration.generated.h"

UCLASS(ClassGroup=(Spycho))
class SPYCHO_API USpychoPenetration : public UActorComponent
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, Category="Ballistics") float InitialEnergy = 100.f;
    UPROPERTY(EditAnywhere, Category="Ballistics") float BaseDamage = 130.f;
    UPROPERTY(EditAnywhere, Category="Ballistics") float Range = 5000.f;
    UPROPERTY(EditAnywhere, Category="Ballistics") int32 MaxPenetrations = 6;
    void Fire(FVector Origin, FVector Direction);
};
