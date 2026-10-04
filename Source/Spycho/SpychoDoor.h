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
    UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> HandleFront;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> HandleBack;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Inset;
    UPROPERTY(VisibleAnywhere) TArray<TObjectPtr<UStaticMeshComponent>> PanelDetails;
    UPROPERTY() TObjectPtr<UPhysicalMaterial> DoorSurface;
    UPROPERTY(Replicated) bool bOpen = false;
    UPROPERTY(Replicated) bool bCareful = false;
    UPROPERTY(Replicated) float SwingAngle=95.f;
    UPROPERTY(Replicated) float PanelYaw=0.f;
    UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Door") float OpenAngle = 95.f;
    void Toggle(bool Carefully,AActor* Operator=nullptr);
    void Reset();
    bool WouldOverlapPawn(float Yaw) const;
    virtual void Tick(float Dt) override;
    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& Out) const override;
};
