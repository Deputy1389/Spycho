#include "SpychoCharacter.h"
#include "SpychoHealth.h"
#include "SpychoHandgun.h"
#include "SpychoPenetration.h"
#include "SpychoGameMode.h"
#include "SpychoGameState.h"
#include "SpychoDoor.h"
#include "SpychoSurface.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

ASpychoCharacter::ASpychoCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    GetCapsuleComponent()->InitCapsuleSize(30.f, 88.f);
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    bUseControllerRotationYaw = true;
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Eyes")); Camera->SetupAttachment(GetCapsuleComponent());
    Camera->SetRelativeLocation(FVector(0,0,64)); Camera->bUsePawnControlRotation = true; Camera->FieldOfView=80.f;
    Camera->PostProcessSettings.bOverride_AutoExposureBias=true; Camera->PostProcessSettings.AutoExposureBias=-3.5f;
    auto* Move = GetCharacterMovement(); Move->MaxWalkSpeed=210.f; Move->MaxWalkSpeedCrouched=85.f;
    Move->GetNavAgentPropertiesRef().bCanCrouch=true; Move->SetCrouchedHalfHeight(52.f); Move->BrakingDecelerationWalking=800.f;
    Move->MaxAcceleration=650.f; Move->bRunPhysicsWithNoController=true;
    Health=CreateDefaultSubobject<USpychoHealth>(TEXT("Health")); Handgun=CreateDefaultSubobject<USpychoHandgun>(TEXT("Handgun")); Penetration=CreateDefaultSubobject<USpychoPenetration>(TEXT("Penetration"));
    ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    ConstructorHelpers::FObjectFinder<UMaterialInterface> Dark(TEXT("/Game/Materials/Dark.Dark"));
    Body=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Silhouette")); Body->SetupAttachment(GetCapsuleComponent());
    Body->SetStaticMesh(Cylinder.Object); Body->SetRelativeScale3D(FVector(0.5f,0.5f,1.45f)); Body->SetMaterial(0,Dark.Object);
    Body->SetCollisionEnabled(ECollisionEnabled::NoCollision); Body->SetOwnerNoSee(true);
    ConstructorHelpers::FObjectFinder<UMaterialInterface> Metal(TEXT("/Game/Materials/WeaponMetal.WeaponMetal"));
    ConstructorHelpers::FObjectFinder<UMaterialInterface> Polymer(TEXT("/Game/Materials/WeaponPolymer.WeaponPolymer"));
    ConstructorHelpers::FObjectFinder<UMaterialInterface> SightPaint(TEXT("/Game/Materials/SightPaint.SightPaint"));
    WeaponRig=CreateDefaultSubobject<USceneComponent>(TEXT("WeaponRig")); WeaponRig->SetupAttachment(Camera);
    WeaponRig->SetRelativeLocation(FVector(34,10,-13));
    // Unscaled parent: all parts use real centimeter dimensions and share the
    // same aiming/recoil transform. The old scaled gun parent distorted its grip.
    auto Part=[&](const TCHAR* Name,FVector Position,FVector Size,UMaterialInterface* Material)
    {
        auto* Mesh=CreateDefaultSubobject<UStaticMeshComponent>(Name); Mesh->SetupAttachment(WeaponRig);
        Mesh->SetStaticMesh(Cube.Object); Mesh->SetMaterial(0,Material); Mesh->SetRelativeLocation(Position); Mesh->SetRelativeScale3D(Size/100.f);
        Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); Mesh->SetOnlyOwnerSee(true); Mesh->SetCastShadow(false);
        return Mesh;
    };
    Gun=Part(TEXT("HandgunView"),FVector(2,0,0),FVector(18,2.8f,3.f),Metal.Object);
    Part(TEXT("Frame"),FVector(2,0,-2.1f),FVector(16,2.5f,1.3f),Polymer.Object);
    auto* Grip=Part(TEXT("Grip"),FVector(-3,0,-6.5f),FVector(3.7f,2.7f,7.8f),Polymer.Object); Grip->SetRelativeRotation(FRotator(-13,0,0));
    Part(TEXT("GuardFront"),FVector(2,0,-4.f),FVector(.5f,2.1f,2.8f),Metal.Object);
    Part(TEXT("GuardBottom"),FVector(-.5f,0,-5.4f),FVector(5.5f,2.1f,.5f),Metal.Object);
    Part(TEXT("Trigger"),FVector(-.5f,0,-3.8f),FVector(.4f,.7f,1.6f),Metal.Object);
    MagazineMesh=Part(TEXT("MagazineBase"),FVector(-3,0,-10.5f),FVector(3.8f,2.8f,.7f),Metal.Object);
    auto* Muzzle=Part(TEXT("Muzzle"),FVector(11.15f,0,0),FVector(.25f,1.15f,1.15f),Polymer.Object);
    Muzzle->SetStaticMesh(Cylinder.Object); Muzzle->SetRelativeScale3D(FVector(.0115f,.0115f,.0025f)); Muzzle->SetRelativeRotation(FRotator(90,0,0));
    // A small gloved hand makes the grip feel held without needing a skeletal rig.
    auto* Hand=Part(TEXT("GlovedHand"),FVector(-4,1.1f,-6.4f),FVector(5.5f,4.2f,6.8f),Polymer.Object);
    ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere")); Hand->SetStaticMesh(Sphere.Object);
    auto* Wrist=Part(TEXT("Wrist"),FVector(-9,2.2f,-9.4f),FVector(9,4,4),Polymer.Object); Wrist->SetRelativeRotation(FRotator(-20,-8,0));
    for (int32 i=0;i<3;++i)
    {
        auto* Sight=Part(*FString::Printf(TEXT("IronSight%d"),i),i==0?FVector(10,0,1.85f):FVector(-6,i==1?-.5f:.5f,1.85f),i==0?FVector(.5f,.26f,.7f):FVector(.5f,.25f,.7f),Metal.Object);
        Sights.Add(Sight);
    }
    Part(TEXT("FrontSightDot"),FVector(9.72f,0,2.02f),FVector(.05f,.17f,.17f),SightPaint.Object);
    MuzzleLight=CreateDefaultSubobject<UPointLightComponent>(TEXT("MuzzleFlash")); MuzzleLight->SetupAttachment(WeaponRig);
    MuzzleLight->SetRelativeLocation(FVector(13,0,0)); MuzzleLight->SetIntensity(1400.f); MuzzleLight->SetAttenuationRadius(170.f);
    MuzzleLight->SetLightColor(FLinearColor(1,.55f,.2f)); MuzzleLight->SetCastShadows(false); MuzzleLight->SetVisibility(false);
}
void ASpychoCharacter::BeginPlay() { Super::BeginPlay(); LastStepPosition=GetActorLocation(); }
void ASpychoCharacter::SetupPlayerInputComponent(UInputComponent* Input)
{
    Super::SetupPlayerInputComponent(Input);
    Input->BindAxis("Forward",this,&ASpychoCharacter::Forward); Input->BindAxis("Right",this,&ASpychoCharacter::Right);
    // MouseY from Unreal is positive when moving up. Bind the raw axes so
    // stale saved axis mappings cannot invert the corrected direction again.
    Input->BindAxisKey(EKeys::MouseX,this,&ASpychoCharacter::Turn); Input->BindAxisKey(EKeys::MouseY,this,&ASpychoCharacter::Look);
    Input->BindAction("Fire",IE_Pressed,this,&ASpychoCharacter::Fire); Input->BindAction("Reload",IE_Pressed,this,&ASpychoCharacter::Reload);
    Input->BindAction("Aim",IE_Pressed,this,&ASpychoCharacter::AimDown); Input->BindAction("Aim",IE_Released,this,&ASpychoCharacter::AimUp);
    Input->BindAction("Interact",IE_Pressed,this,&ASpychoCharacter::Interact);
    Input->BindAction("Crouch",IE_Pressed,this,&ASpychoCharacter::CrouchDown); Input->BindAction("Crouch",IE_Released,this,&ASpychoCharacter::CrouchUp);
    Input->BindAction("Careful",IE_Pressed,this,&ASpychoCharacter::CarefulDown); Input->BindAction("Careful",IE_Released,this,&ASpychoCharacter::CarefulUp);
}
void ASpychoCharacter::Forward(float V) { if (Health->Health>0.f) AddMovementInput(FRotationMatrix(FRotator(0,GetControlRotation().Yaw,0)).GetUnitAxis(EAxis::X),V); }
void ASpychoCharacter::Right(float V) { if (Health->Health>0.f) AddMovementInput(FRotationMatrix(FRotator(0,GetControlRotation().Yaw,0)).GetUnitAxis(EAxis::Y),V); }
void ASpychoCharacter::Turn(float V) { if (Health->Health>0.f) AddControllerYawInput(V*.65f); }
void ASpychoCharacter::Look(float V) { if (Health->Health>0.f) AddControllerPitchInput(V*.65f); }
void ASpychoCharacter::Fire()
{
    auto* GS=GetWorld()->GetGameState<ASpychoGameState>(); double Now=GetWorld()->GetTimeSeconds();
    if (Health->Health<=0.f || !GS || !GS->bRoundActive || Handgun->Magazine<=0 || Handgun->bReloading || Now<LocalReloadUntil) { bBufferedTrigger=false; return; }
    if (Now<NextLocalShot) { bBufferedTrigger=NextLocalShot-Now<=.08; return; }
    bBufferedTrigger=false;
    FRotator Aim=GetControlRotation(); NextLocalShot=Now+Handgun->ShotInterval;
    int32 PredictionKey=++LastPredictedShot; ShotFeedback(); ServerFire(Aim,PredictionKey);
}
void ASpychoCharacter::AimDown() { if (Health->Health>0.f) bAiming=true; }
void ASpychoCharacter::AimUp() { bAiming=false; }
void ASpychoCharacter::Reload()
{
    auto* GS=GetWorld()->GetGameState<ASpychoGameState>();
    if (Health->Health>0.f && GS && GS->bRoundActive && !Handgun->bReloading && Handgun->Magazine<6 && Handgun->Reserve>0)
    { ReloadStarted=GetWorld()->GetTimeSeconds(); LocalReloadUntil=ReloadStarted+Handgun->ReloadSeconds; }
    ServerReload();
}
void ASpychoCharacter::Interact() { ServerInteract(); }
void ASpychoCharacter::CrouchDown() { if (Health->Health>0.f) Crouch(); }
void ASpychoCharacter::CrouchUp() { UnCrouch(); }
void ASpychoCharacter::CarefulDown() { bCareful=true; OnRep_Careful(); ServerCareful(true); }
void ASpychoCharacter::CarefulUp() { bCareful=false; OnRep_Careful(); ServerCareful(false); }
void ASpychoCharacter::ServerCareful_Implementation(bool V) { bCareful=V; OnRep_Careful(); }
void ASpychoCharacter::OnRep_Careful() { GetCharacterMovement()->MaxWalkSpeed=bCareful?105.f:210.f; }
void ASpychoCharacter::ServerFire_Implementation(FRotator Aim,int32 PredictionKey) { Handgun->Fire(Aim,PredictionKey); }
void ASpychoCharacter::ServerReload_Implementation() { Handgun->Reload(); }
void ASpychoCharacter::ServerInteract_Implementation()
{
    auto* GS=GetWorld()->GetGameState<ASpychoGameState>();
    if (Health->Health<=0.f || !GS || !GS->bRoundActive) return;
    FVector Origin=Camera->GetComponentLocation(); FHitResult Hit;
    FCollisionQueryParams Q(SCENE_QUERY_STAT(SpychoInteraction),false,this);
    if (GetWorld()->LineTraceSingleByChannel(Hit,Origin,Origin+GetBaseAimRotation().Vector()*190.f,ECC_Visibility,Q))
        if (auto* Door=Cast<ASpychoDoor>(Hit.GetActor())) Door->Toggle(bCareful||bIsCrouched);
}
void ASpychoCharacter::Tick(float Dt)
{
    Super::Tick(Dt);
    UpdateWeaponPresentation(Dt);
    if (!HasAuthority() || Health->Health<=0.f) return;
    auto* GS=GetWorld()->GetGameState<ASpychoGameState>();
    if (bTestOpponent && Patrol.Num()>0 && GS && GS->bRoundActive && GetWorld()->GetTimeSeconds()>PauseUntil)
    {
        FVector Delta=Patrol[PatrolIndex]-GetActorLocation(); Delta.Z=0;
        if (Delta.Size()<35.f) { PatrolIndex=(PatrolIndex+1)%Patrol.Num(); PauseUntil=GetWorld()->GetTimeSeconds()+FMath::FRandRange(1.4f,3.f); }
        else { AddMovementInput(Delta.GetSafeNormal(),1.f); SetActorRotation(Delta.Rotation()); }
    }
    UpdateFootsteps(Dt);
}
void ASpychoCharacter::UpdateFootsteps(float Dt)
{
    FVector Delta=GetActorLocation()-LastStepPosition; Delta.Z=0; LastStepPosition=GetActorLocation();
    if (!GetCharacterMovement()->IsMovingOnGround() || GetVelocity().Size2D()<15.f) { DistanceSinceStep=0.f; return; }
    DistanceSinceStep+=FMath::Min(Delta.Size(),GetVelocity().Size2D()*Dt*2.f);
    float Stride=bIsCrouched?70.f:(bCareful?95.f:125.f);
    if (DistanceSinceStep<Stride) return;
    DistanceSinceStep-=Stride;
    FHitResult Floor; FCollisionQueryParams Q(SCENE_QUERY_STAT(SpychoFloor),false,this); Q.bReturnPhysicalMaterial=true;
    GetWorld()->LineTraceSingleByChannel(Floor,GetActorLocation(),GetActorLocation()-FVector(0,0,130),ECC_Visibility,Q);
    auto* Surface=Cast<USpychoSurface>(Floor.PhysMaterial.Get());
    ESpychoNoise Kind=ESpychoNoise::Wood;
    if (Surface && Surface->SurfaceType==SurfaceType3) Kind=ESpychoNoise::Carpet;
    if (Surface && Surface->SurfaceType==SurfaceType4) Kind=ESpychoNoise::Tile;
    float Gain=(bIsCrouched?0.05f:(bCareful?0.09f:0.23f))*(Surface?Surface->FootstepGain:1.f);
    if (auto* GS=GetWorld()->GetGameState<ASpychoGameState>()) GS->Noise(Kind,Floor.bBlockingHit?Floor.ImpactPoint:GetActorLocation(),Gain);
}
void ASpychoCharacter::UpdateWeaponPresentation(float Dt)
{
    float Now=GetWorld()->GetTimeSeconds(); bool Reloading=Handgun->bReloading || Now<LocalReloadUntil;
    // Once accepted inside the buffer window, survive a slow frame until ready.
    if (IsLocallyControlled() && bBufferedTrigger && Now>=NextLocalShot) Fire();
    if (Reloading && !bWasReloading && ReloadStarted<Now-Handgun->ReloadSeconds) ReloadStarted=Now;
    bWasReloading=Reloading;
    auto Blend=[Dt](float Speed){return 1.f-FMath::Exp(-Speed*Dt);};
    AimAlpha=FMath::Lerp(AimAlpha,bAiming&&!Reloading?1.f:0.f,Blend(14.f));
    GunKick*=FMath::Exp(-17.f*Dt); GunRise*=FMath::Exp(-14.f*Dt); SlideKick*=FMath::Exp(-45.f*Dt);
    if (IsLocallyControlled() && Controller && CameraRecovery>0.001f)
    {
        float Remaining=CameraRecovery*FMath::Exp(-9.f*Dt); FRotator R=Controller->GetControlRotation();
        R.Pitch=FRotator::NormalizeAxis(R.Pitch)-(CameraRecovery-Remaining); Controller->SetControlRotation(R); CameraRecovery=Remaining;
    }
    float Speed=GetVelocity().Size2D(); MovingAlpha=FMath::Lerp(MovingAlpha,FMath::Clamp(Speed/210.f,0.f,1.f),Blend(8.f));
    WalkPhase+=Speed*Dt/125.f*2.f*PI;
    float Bob=MovingAlpha*(1.f-.9f*AimAlpha);
    float ReloadAlpha=Reloading?FMath::Clamp((Now-ReloadStarted)/Handgun->ReloadSeconds,0.f,1.f):0.f;
    float ReloadTilt=Reloading?FMath::Min(1.f,ReloadAlpha/.12f)*FMath::Clamp((1.f-ReloadAlpha)/.18f,0.f,1.f):0.f;
    FVector Pos=FMath::Lerp(FVector(34,10,-13),FVector(36,0,-2.2f),AimAlpha);
    Pos+=FVector(-GunKick,.3f*FMath::Sin(WalkPhase)*Bob,.25f*FMath::Cos(WalkPhase*2)*Bob-ReloadTilt*5.f);
    WeaponRig->SetRelativeLocation(Pos);
    WeaponRig->SetRelativeRotation(FRotator((1.f-AimAlpha)*-3.f+GunRise-ReloadTilt*22.f,(1.f-AimAlpha)*-4.f,ReloadTilt*45.f));
    Gun->SetRelativeLocation(FVector(2-SlideKick,0,0));
    float MagazineDrop=Reloading?FMath::Sin(PI*FMath::Clamp((ReloadAlpha-.16f)/.65f,0.f,1.f))*11.f:0.f;
    MagazineMesh->SetRelativeLocation(FVector(-3,0,-10.5f-MagazineDrop));
    Camera->FieldOfView=FMath::Lerp(80.f,65.f,AimAlpha);
    MuzzleLight->SetVisibility(Now<FlashUntil && Health->Health>0.f);
}
void ASpychoCharacter::ShotFeedback()
{
    ++ShotFeedbackCount;
    GunKick=2.8f; GunRise=FMath::Lerp(7.f,4.f,AimAlpha); SlideKick=1.4f; FlashUntil=GetWorld()->GetTimeSeconds()+.035f;
    if (IsLocallyControlled() && Controller)
    {
        float Impulse=FMath::Lerp(1.1f,.65f,AimAlpha); FRotator R=Controller->GetControlRotation();
        R.Pitch=FRotator::NormalizeAxis(R.Pitch)+Impulse; Controller->SetControlRotation(R); CameraRecovery+=Impulse*.85f;
    }
    if (auto* GS=GetWorld()->GetGameState<ASpychoGameState>()) GS->Noise_Implementation(ESpychoNoise::Gunshot,GetActorLocation()+FVector(0,0,60),2.8f);
}
void ASpychoCharacter::ConfirmShotFeedback(int32 PredictionKey)
{
    // Cosmetic prediction stays immediate; accepted shots reach other players
    // reliably, and the owning client never plays recoil/audio a second time.
    if (IsLocallyControlled() && PredictionKey>0 && PredictionKey<=LastPredictedShot) return;
    ShotFeedback();
}
void ASpychoCharacter::Die()
{
    if (bDeathHandled) return; bDeathHandled=true;
    GetCharacterMovement()->DisableMovement(); GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Body->SetRelativeRotation(FRotator(80,0,0)); Body->SetRelativeLocation(FVector(0,0,-65)); WeaponRig->SetVisibility(false,true); AimUp();
    if (HasAuthority()) { Handgun->CancelReload(); if (auto* GM=GetWorld()->GetAuthGameMode<ASpychoGameMode>()) GM->OnDeath(this); }
}
void ASpychoCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps); DOREPLIFETIME(ASpychoCharacter,bTestOpponent); DOREPLIFETIME(ASpychoCharacter,bCareful);
}
