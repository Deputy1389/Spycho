#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpychoDistraction.generated.h"
class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;
UCLASS()
class SPYCHO_API ASpychoDistraction : public AActor
{
    GENERATED_BODY()
public:
    ASpychoDistraction();
    UPROPERTY(VisibleAnywhere) TObjectPtr<USphereComponent> Collision;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Coin;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UProjectileMovementComponent> Movement;
    int32 AudibleBounces=0;
    FVector LastLanding=FVector::ZeroVector;
    FVector FirstLanding=FVector::ZeroVector;
    void Launch(FVector Velocity);
    virtual void BeginPlay() override;
private:
    float LastSound=-100.f;
    UFUNCTION() void Bounce(const FHitResult& Hit,const FVector& ImpactVelocity);
};
