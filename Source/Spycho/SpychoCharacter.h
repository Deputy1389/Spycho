#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SpychoCharacter.generated.h"
class UCameraComponent;
class UStaticMeshComponent;
class USpychoHealth;
class USpychoHandgun;
class USpychoPenetration;
class UPointLightComponent;
class USkeletalMeshComponent;
class UAnimSequence;

UCLASS()
class SPYCHO_API ASpychoCharacter : public ACharacter
{
    GENERATED_BODY()
public:
    ASpychoCharacter();
    UPROPERTY(VisibleAnywhere) TObjectPtr<UCameraComponent> Camera;
    UPROPERTY(VisibleAnywhere) TObjectPtr<USpychoHealth> Health;
    UPROPERTY(VisibleAnywhere) TObjectPtr<USpychoHandgun> Handgun;
    UPROPERTY(VisibleAnywhere) TObjectPtr<USpychoPenetration> Penetration;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Gun;
    UPROPERTY(VisibleAnywhere) TObjectPtr<USceneComponent> WeaponRig;
    UPROPERTY() TObjectPtr<UPointLightComponent> MuzzleLight;
    UPROPERTY() TObjectPtr<USkeletalMeshComponent> FirstPersonArms;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> WorldGun;
    UPROPERTY() TObjectPtr<UAnimSequence> IdleAnimation;
    UPROPERTY() TObjectPtr<UAnimSequence> WalkAnimation;
    UPROPERTY() TObjectPtr<UAnimSequence> RunAnimation;
    UPROPERTY(Replicated) bool bTestOpponent = false;
    UPROPERTY(ReplicatedUsing=OnRep_Careful) bool bCareful = false;
    UPROPERTY(ReplicatedUsing=OnRep_Careful) bool bSprinting = false;
    void Die();
    void ShotFeedback();
    void ConfirmShotFeedback(int32 PredictionKey);
    float GetAimAlpha() const { return AimAlpha; }
    int32 GetShotFeedbackCount() const { return ShotFeedbackCount; }
    int32 GetFootstepCount() const { return FootstepCount; }
    virtual void Tick(float DeltaSeconds) override;
    virtual void SetupPlayerInputComponent(UInputComponent* Input) override;
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& Out) const override;
    UFUNCTION(Server, Reliable) void ServerFire(FRotator Aim, int32 PredictionKey);
    UFUNCTION(Server, Reliable) void ServerReload();
    UFUNCTION(Server, Reliable) void ServerInteract();
    UFUNCTION(Server, Reliable) void ServerCareful(bool Value);
    UFUNCTION(Server, Reliable) void ServerSprint(bool Value);
    UFUNCTION() void OnRep_Careful();
private:
    void Forward(float Value); void Right(float Value); void Turn(float Value); void Look(float Value);
    void Fire(); void Reload(); void Interact(); void CrouchDown(); void CrouchUp(); void CarefulDown(); void CarefulUp();
    void UpdateFootsteps(float DeltaSeconds);
    float DistanceSinceStep = 0.f;
    FVector LastStepPosition;
    bool bDeathHandled = false;
    void UpdateWeaponPresentation(float DeltaSeconds);
    void SprintDown(); void SprintUp();
    int32 BodyAnimation = -1;
    int32 FootstepCount = 0;
    float GunKick = 0.f;
    float GunRise = 0.f;
    float SlideKick = 0.f;
    float CameraRecovery = 0.f;
    float AimAlpha = 0.f;
    float WalkPhase = 0.f;
    float MovingAlpha = 0.f;
    float FlashUntil = 0.f;
    float LocalReloadUntil = 0.f;
    float ReloadStarted = -100.f;
    bool bWasReloading = false;
    double NextLocalShot = 0;
    bool bBufferedTrigger = false;
    int32 LastPredictedShot = 0;
    int32 ShotFeedbackCount = 0;
    bool bAiming = false;
    void AimDown(); void AimUp();
};
