#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpychoDoor.generated.h"
class UStaticMeshComponent;
class UPhysicalMaterial;
UCLASS()
class SPYCHO_API ASpychoDoor : public AActor
{
    GENERATED_BODY()
public:
    ASpychoDoor();
    UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Panel;
    UPROPERTY(VisibleAnywhere) TObjectPtr<USceneComponent> Swing;
    UPROPERTY() TObjectPtr<UPhysicalMaterial> DoorSurface;
    UPROPERTY(Replicated) bool bOpen = false;
    UPROPERTY(Replicated) bool bCareful = false;
    UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Door") float OpenAngle = 95.f;
    void Toggle(bool Carefully,AActor* Operator=nullptr);
    void Reset();
    virtual void Tick(float Dt) override;
    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& Out) const override;
};
