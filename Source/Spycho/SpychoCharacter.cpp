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
#include "Components/SkeletalMeshComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "SpychoArmsAnim.h"
#include "SpychoPlayerController.h"
#include "SpychoOptions.h"
#include "Animation/AnimSequence.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "EngineUtils.h"
#include "SpychoDistraction.h"
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
    Camera->PostProcessSettings.bOverride_AutoExposureBias=true; Camera->PostProcessSettings.AutoExposureBias=-1.6f;
    auto* Move = GetCharacterMovement(); Move->MaxWalkSpeed=210.f;
    Move->GetNavAgentPropertiesRef().bCanCrouch=false; Move->BrakingDecelerationWalking=1600.f;
    Move->MaxAcceleration=1200.f;Move->GroundFriction=8.f;Move->BrakingFrictionFactor=1.f;Move->bRunPhysicsWithNoController=true;
    Health=CreateDefaultSubobject<USpychoHealth>(TEXT("Health")); Handgun=CreateDefaultSubobject<USpychoHandgun>(TEXT("Handgun")); Penetration=CreateDefaultSubobject<USpychoPenetration>(TEXT("Penetration"));
    ConstructorHelpers::FObjectFinder<USkeletalMesh> Manny(TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple"));
    ConstructorHelpers::FObjectFinder<UAnimSequence> Idle(TEXT("/Game/Characters/Mannequins/Anims/Pistol/MF_Pistol_Idle_ADS"));
    ConstructorHelpers::FObjectFinder<UAnimSequence> Walk(TEXT("/Game/Characters/Mannequins/Anims/Pistol/Walk/MF_Pistol_Walk_Fwd"));
    ConstructorHelpers::FObjectFinder<UAnimSequence> Run(TEXT("/Game/Characters/Mannequins/Anims/Pistol/Jog/MF_Pistol_Jog_Fwd"));
    IdleAnimation=Idle.Object;WalkAnimation=Walk.Object;RunAnimation=Run.Object;
    ConstructorHelpers::FObjectFinder<UAnimSequence> FireAnim(TEXT("/Game/Characters/Mannequins/Anims/Pistol/MM_Pistol_Fire"));
    ConstructorHelpers::FObjectFinder<UAnimSequence> ReloadAnim(TEXT("/Game/Characters/Mannequins/Anims/Pistol/MM_Pistol_Reload"));
    FireAnimation=FireAnim.Object;ReloadAnimation=ReloadAnim.Object;
    GetMesh()->SetSkeletalMesh(Manny.Object);GetMesh()->SetRelativeLocation(FVector(0,0,-88));GetMesh()->SetRelativeRotation(FRotator(0,-90,0));
    GetMesh()->SetOwnerNoSee(true);GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetMesh()->SetAnimInstanceClass(USpychoArmsAnim::StaticClass());
    WeaponRig=CreateDefaultSubobject<USceneComponent>(TEXT("WeaponRig")); WeaponRig->SetupAttachment(Camera);
    WeaponRig->SetRelativeLocation(FVector(30,11,-23));
    ConstructorHelpers::FObjectFinder<USkeletalMesh> Pistol(TEXT("/Game/Weapons/Pistol/Meshes/SKM_Pistol"));
    Gun=CreateDefaultSubobject<UPoseableMeshComponent>(TEXT("HandgunView"));
    Gun->SetSkeletalMesh(Pistol.Object);
    Gun->SetOnlyOwnerSee(true);Gun->SetCastShadow(false);Gun->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    FirstPersonArms=CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonHands"));FirstPersonArms->SetupAttachment(WeaponRig);
    FirstPersonArms->SetSkeletalMesh(Manny.Object);FirstPersonArms->SetRelativeRotation(FRotator(0,-90,0));
    FirstPersonArms->SetOnlyOwnerSee(true);FirstPersonArms->SetCastShadow(false);FirstPersonArms->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    FirstPersonArms->SetAnimInstanceClass(USpychoArmsAnim::StaticClass());
    FirstPersonArms->VisibilityBasedAnimTickOption=EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
    // +Y is the barrel axis. Use the hand socket's orientation, then move this
    // compact pistol back from the generic weapon socket into the palm.
    Gun->SetupAttachment(FirstPersonArms,TEXT("HandGrip_R"));
    Gun->SetRelativeLocation(FVector(0,-5,-1));
    WorldGun=CreateDefaultSubobject<UPoseableMeshComponent>(TEXT("HeldPistol"));WorldGun->SetupAttachment(GetMesh(),TEXT("HandGrip_R"));
    WorldGun->SetSkeletalMesh(Pistol.Object);WorldGun->SetOwnerNoSee(true);
    WorldGun->SetRelativeLocation(FVector(0,-5,-1));
    WorldGun->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ReloadMagazine=CreateDefaultSubobject<UPoseableMeshComponent>(TEXT("ReloadMagazine"));ReloadMagazine->SetupAttachment(FirstPersonArms,TEXT("HandGrip_L"));
    ReloadMagazine->SetSkeletalMesh(Pistol.Object);ReloadMagazine->SetOnlyOwnerSee(true);ReloadMagazine->SetCastShadow(false);
    ReloadMagazine->SetCollisionEnabled(ECollisionEnabled::NoCollision);ReloadMagazine->SetVisibility(false);
    MuzzleLight=CreateDefaultSubobject<UPointLightComponent>(TEXT("MuzzleFlash")); MuzzleLight->SetupAttachment(Gun);
    MuzzleLight->SetRelativeLocation(FVector(0,20,9)); MuzzleLight->SetIntensity(2200.f); MuzzleLight->SetAttenuationRadius(170.f);
    MuzzleLight->SetLightColor(FLinearColor(1,.55f,.2f)); MuzzleLight->SetCastShadows(false); MuzzleLight->SetVisibility(false);
}
void ASpychoCharacter::BeginPlay()
{
    Super::BeginPlay();LastStepPosition=GetActorLocation();
    FirstPersonArms->HideBoneByName(TEXT("head"),EPhysBodyOp::PBO_None);
    FirstPersonArms->HideBoneByName(TEXT("thigh_l"),EPhysBodyOp::PBO_None);FirstPersonArms->HideBoneByName(TEXT("thigh_r"),EPhysBodyOp::PBO_None);
    FirstPersonArms->TickAnimation(0.f,false);FirstPersonArms->RefreshBoneTransforms();
    FTransform Grip=FirstPersonArms->GetSocketTransform(TEXT("HandGrip_R"),RTS_Component);
    FTransform Alignment=Grip.Inverse()*FTransform(FRotator(0,-90,0));
    FirstPersonArms->SetRelativeRotation(Alignment.GetRotation());
    FirstPersonArms->RegisterOnBoneTransformsFinalizedDelegate(FOnBoneTransformsFinalizedMultiCast::FDelegate::CreateUObject(this,&ASpychoCharacter::AnchorFirstPersonGrip));
    AnchorFirstPersonGrip();
    Gun->RefreshBoneTransforms();WorldGun->RefreshBoneTransforms();
    SlideRest=Gun->GetBoneTransformByName(TEXT("Slide"),EBoneSpaces::ComponentSpace);
    MagazineRest=Gun->GetBoneTransformByName(TEXT("Magazine"),EBoneSpaces::ComponentSpace);
    TriggerRest=Gun->GetBoneTransformByName(TEXT("Trigger"),EBoneSpaces::ComponentSpace);
    // Keep the parent transform invertible while shrinking its geometry. The
    // magazine is a child of Grip, so a zero parent scale would collapse it too.
    for (const TCHAR* Bone:{TEXT("Grip"),TEXT("Slide"),TEXT("Trigger"),TEXT("Barrel"),TEXT("Door")}) ReloadMagazine->SetBoneScaleByName(Bone,FVector(.0001f),EBoneSpaces::ComponentSpace);
    ReloadMagazine->SetBoneTransformByName(TEXT("Magazine"),MagazineRest,EBoneSpaces::ComponentSpace);
    ReloadMagazine->SetRelativeLocation(-MagazineRest.GetLocation()+FVector(0,0,-2));
}
bool ASpychoCharacter::IsReloadingPresentation() const { return Handgun->bReloading||GetWorld()->GetTimeSeconds()<LocalReloadUntil; }
float ASpychoCharacter::GetReloadProgress() const { return FMath::Clamp((GetWorld()->GetTimeSeconds()-ReloadStarted)/Handgun->ReloadSeconds,0.f,1.f); }
float ASpychoCharacter::GetShotProgress() const { return FMath::Clamp(1.f-(ArmShotUntil-GetWorld()->GetTimeSeconds())/.18f,0.f,1.f); }
void ASpychoCharacter::ResetHeldInput()
{
    bAiming=false;bBufferedTrigger=false;bCareful=false;bSprinting=false;OnRep_Careful();
    if (IsLocallyControlled()) { ServerCareful(false);ServerSprint(false); }
}
void ASpychoCharacter::AnchorFirstPersonGrip()
{
    // Run after animation evaluation so the gun and fingers use the same pose.
    // Translate the whole arm assembly; never move the pistol out of its socket.
    FVector Grip=FirstPersonArms->GetSocketTransform(TEXT("HandGrip_R"),RTS_Component).GetLocation();
    FirstPersonArms->SetRelativeLocation(-FirstPersonArms->GetRelativeRotation().RotateVector(Grip));
}
void ASpychoCharacter::SetupPlayerInputComponent(UInputComponent* Input)
{
    Super::SetupPlayerInputComponent(Input);
    Input->BindAxis("Forward",this,&ASpychoCharacter::Forward); Input->BindAxis("Right",this,&ASpychoCharacter::Right);
    // MouseY from Unreal is positive when moving up. Bind the raw axes so
    // stale saved axis mappings cannot invert the corrected direction again.
    Input->BindAxisKey(EKeys::MouseX,this,&ASpychoCharacter::Turn); Input->BindAxisKey(EKeys::MouseY,this,&ASpychoCharacter::Look);
    Input->BindKey(EKeys::LeftMouseButton,IE_Pressed,this,&ASpychoCharacter::Fire); Input->BindAction("Reload",IE_Pressed,this,&ASpychoCharacter::Reload);
    Input->BindKey(EKeys::RightMouseButton,IE_Pressed,this,&ASpychoCharacter::AimDown); Input->BindKey(EKeys::RightMouseButton,IE_Released,this,&ASpychoCharacter::AimUp);
    Input->BindAction("Interact",IE_Pressed,this,&ASpychoCharacter::Interact);
    Input->BindKey(EKeys::Q,IE_Pressed,this,&ASpychoCharacter::ThrowCoin);
    Input->BindKey(EKeys::LeftControl,IE_Pressed,this,&ASpychoCharacter::CarefulDown); Input->BindKey(EKeys::LeftControl,IE_Released,this,&ASpychoCharacter::CarefulUp);
    Input->BindKey(EKeys::LeftAlt,IE_Pressed,this,&ASpychoCharacter::CarefulDown); Input->BindKey(EKeys::LeftAlt,IE_Released,this,&ASpychoCharacter::CarefulUp);
    Input->BindKey(EKeys::LeftShift,IE_Pressed,this,&ASpychoCharacter::SprintDown); Input->BindKey(EKeys::LeftShift,IE_Released,this,&ASpychoCharacter::SprintUp);
}
void ASpychoCharacter::Forward(float V) { if (Health->Health>0.f) AddMovementInput(FRotationMatrix(FRotator(0,GetControlRotation().Yaw,0)).GetUnitAxis(EAxis::X),V); }
void ASpychoCharacter::Right(float V) { if (Health->Health>0.f) AddMovementInput(FRotationMatrix(FRotator(0,GetControlRotation().Yaw,0)).GetUnitAxis(EAxis::Y),V); }
void ASpychoCharacter::Turn(float V) { if (Health->Health>0.f) { auto* PC=Cast<ASpychoPlayerController>(Controller);AddControllerYawInput(V*.65f*(PC&&PC->Options?PC->Options->Sensitivity:1.f)); } }
void ASpychoCharacter::Look(float V) { if (Health->Health>0.f) { auto* PC=Cast<ASpychoPlayerController>(Controller);AddControllerPitchInput(V*.65f*(PC&&PC->Options?PC->Options->Sensitivity:1.f)); } }
void ASpychoCharacter::Fire()
{
    auto* GS=GetWorld()->GetGameState<ASpychoGameState>(); double Now=GetWorld()->GetTimeSeconds();
    auto* PC=Cast<ASpychoPlayerController>(Controller);
    if ((PC&&PC->bMenuOpen)||Health->Health<=0.f || !GS || !GS->CanShoot() || Handgun->Magazine<=0 || Handgun->bReloading || Now<LocalReloadUntil) { bBufferedTrigger=false; return; }
    if (Now<NextLocalShot) { bBufferedTrigger=true; return; }
    bBufferedTrigger=false;
    FRotator Aim=GetControlRotation(); NextLocalShot=Now+Handgun->ShotInterval;
    int32 PredictionKey=++LastPredictedShot; ShotFeedback(); ServerFire(Aim,PredictionKey);
}
void ASpychoCharacter::AimDown() { if (Health->Health>0.f) bAiming=true; }
void ASpychoCharacter::AimUp() { bAiming=false; }
void ASpychoCharacter::Reload()
{
    if (auto* PC=Cast<ASpychoPlayerController>(Controller)) if (PC->bMenuOpen) return;
    if (GetWorld()->GetTimeSeconds()<LocalReloadUntil) return;
    auto* GS=GetWorld()->GetGameState<ASpychoGameState>();
    if (Health->Health>0.f && GS && GS->bRoundActive && !Handgun->bReloading && Handgun->Magazine<6 && Handgun->Reserve>0)
    { ReloadStarted=GetWorld()->GetTimeSeconds(); LocalReloadUntil=ReloadStarted+Handgun->ReloadSeconds; }
    ServerReload();
}
void ASpychoCharacter::Interact() { if (auto* PC=Cast<ASpychoPlayerController>(Controller)) if (PC->bMenuOpen) return;ServerInteract(); }
void ASpychoCharacter::CarefulDown() { bCareful=true; OnRep_Careful(); ServerCareful(true); }
void ASpychoCharacter::CarefulUp()
{
    auto* PC=Cast<APlayerController>(Controller);
    bCareful=PC&&(PC->IsInputKeyDown(EKeys::LeftControl)||PC->IsInputKeyDown(EKeys::LeftAlt));
    OnRep_Careful();ServerCareful(bCareful);
}
void ASpychoCharacter::ServerCareful_Implementation(bool V) { bCareful=V; OnRep_Careful(); }
void ASpychoCharacter::SprintDown() { bSprinting=true;OnRep_Careful();ServerSprint(true); }
void ASpychoCharacter::SprintUp() { bSprinting=false;OnRep_Careful();ServerSprint(false); }
void ASpychoCharacter::ServerSprint_Implementation(bool V) { bSprinting=V;OnRep_Careful(); }
void ASpychoCharacter::OnRep_Careful() { GetCharacterMovement()->MaxWalkSpeed=bCareful?95.f:(bSprinting?420.f:210.f); }
void ASpychoCharacter::ServerFire_Implementation(FRotator Aim,int32 PredictionKey) { Handgun->Fire(Aim,PredictionKey); }
void ASpychoCharacter::ServerReload_Implementation() { Handgun->Reload(); }
void ASpychoCharacter::ServerInteract_Implementation()
{
    auto* GS=GetWorld()->GetGameState<ASpychoGameState>();
    if (Health->Health<=0.f || !GS) return;
    if (auto* Door=GetUsableDoor()) Door->Toggle(bCareful,this);
}
ASpychoDoor* ASpychoCharacter::GetUsableDoor() const
{
    if (Health->Health<=0.f) return nullptr;
    FVector Origin=Camera->GetComponentLocation(),Forward=GetBaseAimRotation().Vector();
    ASpychoDoor* Best=nullptr;float BestScore=MAX_flt;
    for (TActorIterator<ASpychoDoor> It(GetWorld());It;++It)
    {
        // Pick an interior point nearest the eyes, so touching a door or
        // looking near its edge doesn't force the player to aim at waist height.
        FTransform PanelTransform=It->Panel->GetComponentTransform();
        FVector LocalEyes=PanelTransform.InverseTransformPosition(Origin);
        FVector Target=PanelTransform.TransformPosition(FVector(0,FMath::Clamp(LocalEyes.Y,-45.f,45.f),FMath::Clamp(LocalEyes.Z,-45.f,45.f)));
        FVector Delta=Target-Origin;
        float Distance=Delta.Size(),Facing=FVector::DotProduct(Forward,Delta.GetSafeNormal());
        if (Distance>240.f || Facing<.65f) continue;
        FHitResult Hit;FCollisionQueryParams Q(SCENE_QUERY_STAT(SpychoInteraction),false,this);
        if (!GetWorld()->LineTraceSingleByChannel(Hit,Origin,Target,ECC_Visibility,Q) || Hit.GetActor()!=*It) continue;
        float Score=Distance+(1.f-Facing)*150.f;
        if (Score<BestScore) { Best=*It;BestScore=Score; }
    }
    return Best;
}
void ASpychoCharacter::ThrowCoin()
{
    if (auto* PC=Cast<ASpychoPlayerController>(Controller)) if (PC->bMenuOpen) return;
    ServerThrowCoin(GetControlRotation());
}
void ASpychoCharacter::ServerThrowCoin_Implementation(FRotator Aim)
{
    auto* GS=GetWorld()->GetGameState<ASpychoGameState>();float Now=GetWorld()->GetTimeSeconds();
    if (!GS||!GS->bRoundActive||Coins<=0||Health->Health<=0.f||Now<NextCoin||Aim.ContainsNaN()) return;
    Aim.Pitch=FMath::Clamp(FRotator::NormalizeAxis(Aim.Pitch),-65.f,65.f);Aim.Roll=0;
    FActorSpawnParameters P;P.Owner=this;P.Instigator=this;P.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    auto* Coin=GetWorld()->SpawnActor<ASpychoDistraction>(Camera->GetComponentLocation()+Aim.Vector()*18.f,Aim,P);
    if (Coin) { --Coins;NextCoin=Now+.8f;Coin->Launch(Aim.Vector()*620.f+FVector(0,0,115)); }
}
void ASpychoCharacter::Tick(float Dt)
{
    Super::Tick(Dt);
    UpdateWeaponPresentation(Dt);
    if (!HasAuthority() || Health->Health<=0.f) return;
    UpdateFootsteps(Dt);
}
void ASpychoCharacter::UpdateFootsteps(float Dt)
{
    FVector Delta=GetActorLocation()-LastStepPosition; Delta.Z=0; LastStepPosition=GetActorLocation();
    if (!GetCharacterMovement()->IsMovingOnGround() || GetVelocity().Size2D()<15.f) { DistanceSinceStep=0.f; return; }
    DistanceSinceStep+=FMath::Min(Delta.Size(),GetVelocity().Size2D()*Dt*2.f);
    float Stride=bCareful?95.f:(bSprinting?150.f:125.f);
    if (DistanceSinceStep<Stride) return;
    DistanceSinceStep-=Stride;
    ++FootstepCount;
    FHitResult Floor; FCollisionQueryParams Q(SCENE_QUERY_STAT(SpychoFloor),false,this); Q.bReturnPhysicalMaterial=true;
    GetWorld()->LineTraceSingleByChannel(Floor,GetActorLocation(),GetActorLocation()-FVector(0,0,130),ECC_Visibility,Q);
    auto* Surface=Cast<USpychoSurface>(Floor.PhysMaterial.Get());
    ESpychoNoise Kind=ESpychoNoise::Wood;
    if (Surface && Surface->SurfaceType==SurfaceType3) Kind=ESpychoNoise::Carpet;
    if (Surface && Surface->SurfaceType==SurfaceType4) Kind=ESpychoNoise::Tile;
    float Gain=(bCareful?0.055f:(bSprinting?.55f:.23f))*(Surface?Surface->FootstepGain:1.f);
    if (auto* GS=GetWorld()->GetGameState<ASpychoGameState>())
    {
        FVector Location=Floor.bBlockingHit?Floor.ImpactPoint:GetActorLocation();GS->Noise(Kind,Location,Gain);GS->NotifyHearing(Kind,Location,Gain,this);
    }
}
void ASpychoCharacter::UpdateWeaponPresentation(float Dt)
{
    float Now=GetWorld()->GetTimeSeconds(); bool Reloading=Handgun->bReloading || Now<LocalReloadUntil;
    // Queue at most one follow-up click throughout cooldown, including slow frames.
    if (IsLocallyControlled() && bBufferedTrigger && Now>=NextLocalShot) Fire();
    if (Reloading && !bWasReloading && ReloadStarted<Now-Handgun->ReloadSeconds) ReloadStarted=Now;
    bWasReloading=Reloading;
    auto Blend=[Dt](float Speed){return 1.f-FMath::Exp(-Speed*Dt);};
    AimAlpha=FMath::Lerp(AimAlpha,bAiming&&!Reloading?1.f:0.f,Blend(14.f));
    GunKick*=FMath::Exp(-17.f*Dt); GunRise*=FMath::Exp(-14.f*Dt);
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
    FHitResult Wall;FCollisionQueryParams WQ(SCENE_QUERY_STAT(SpychoWeaponClearance),false,this);
    for (TActorIterator<ASpychoCharacter> It(GetWorld());It;++It) WQ.AddIgnoredActor(*It);
    FVector Eyes=Camera->GetComponentLocation();
    bool Blocked=GetWorld()->SweepSingleByChannel(Wall,Eyes,Eyes+Camera->GetForwardVector()*70.f,FQuat::Identity,ECC_Visibility,FCollisionShape::MakeSphere(5.f),WQ);
    WallLowering=FMath::Lerp(WallLowering,Blocked?FMath::Clamp((70.f-Wall.Distance)/45.f,0.f,1.f):0.f,Blend(18.f));
    FRotator Aim=GetControlRotation();float YawDelta=FMath::FindDeltaAngleDegrees(PreviousAim.Yaw,Aim.Yaw);PreviousAim=Aim;
    TurnSway=FMath::Lerp(TurnSway,FMath::Clamp(YawDelta,-3.f,3.f),Blend(10.f));
    FVector Pos=FMath::Lerp(FVector(42,12,-21),FVector(46,0,-11.6f),AimAlpha);
    Pos=FMath::Lerp(Pos,FVector(44,10,-16),ReloadTilt);
    Pos+=FVector(-GunKick-WallLowering*22.f,.22f*FMath::Sin(WalkPhase)*Bob-TurnSway*.18f*(1.f-AimAlpha),.18f*FMath::Cos(WalkPhase*2)*Bob-WallLowering*10.f);
    WeaponRig->SetRelativeLocation(Pos);
    WeaponRig->SetRelativeRotation(FRotator((1.f-AimAlpha)*-3.f+GunRise+ReloadTilt*6.f-WallLowering*18.f,(1.f-AimAlpha)*-4.f-ReloadTilt*8.f,ReloadTilt*15.f-TurnSway*(1.f-AimAlpha)));
    float Shot=GetShotProgress();
    SlideTravel=Shot<.48f?FMath::Sin(Shot/.48f*PI)*1.4f:0.f;
    if (!Reloading&&Handgun->Magazine==0) SlideTravel=1.4f;
    float MagDrop=Reloading?(ReloadAlpha<.22f?FMath::Clamp((ReloadAlpha-.12f)/.1f,0.f,1.f):FMath::Clamp((.8f-ReloadAlpha)/.18f,0.f,1.f)):0.f;
    bool Removed=Reloading&&ReloadAlpha>=.22f&&ReloadAlpha<.62f;
    for (auto* Weapon:{Gun.Get(),WorldGun.Get()})
    {
        FTransform Slide=SlideRest;Slide.AddToTranslation(FVector(0,-SlideTravel,0));Weapon->SetBoneTransformByName(TEXT("Slide"),Slide,EBoneSpaces::ComponentSpace);
        FTransform Mag=MagazineRest;Mag.AddToTranslation(FVector(0,0,-MagDrop*10.f));if (Removed) Mag.SetScale3D(FVector::ZeroVector);
        Weapon->SetBoneTransformByName(TEXT("Magazine"),Mag,EBoneSpaces::ComponentSpace);
        FTransform Trigger=TriggerRest;Trigger.AddToTranslation(FVector(0,-.2f*(1.f-Shot),0));Weapon->SetBoneTransformByName(TEXT("Trigger"),Trigger,EBoneSpaces::ComponentSpace);
    }
    ReloadMagazine->SetVisibility(Removed&&Health->Health>0.f);
    Camera->FieldOfView=FMath::Lerp(80.f,65.f,AimAlpha);
    MuzzleLight->SetVisibility(Now<FlashUntil && Health->Health>0.f);
}
void ASpychoCharacter::ShotFeedback()
{
    ++ShotFeedbackCount;
    ArmShotUntil=GetWorld()->GetTimeSeconds()+.18f;
    GunKick=1.6f; GunRise=FMath::Lerp(4.f,2.5f,AimAlpha); FlashUntil=GetWorld()->GetTimeSeconds()+.035f;
    if (IsLocallyControlled() && Controller)
    {
        float Impulse=FMath::Lerp(1.1f,.65f,AimAlpha); FRotator R=Controller->GetControlRotation();
        R.Pitch=FRotator::NormalizeAxis(R.Pitch)+Impulse; Controller->SetControlRotation(R); CameraRecovery+=Impulse*.85f;
    }
    if (auto* GS=GetWorld()->GetGameState<ASpychoGameState>()) GS->Noise_Implementation(ESpychoNoise::Gunshot,GetActorLocation()+FVector(0,0,60),2.8f);
    if (GetNetMode()!=NM_DedicatedServer)
    {
        AActor* Shell=GetWorld()->SpawnActor<AActor>();auto* Casing=NewObject<UStaticMeshComponent>(Shell);Shell->SetRootComponent(Casing);
        Casing->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cylinder")));
        Casing->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Materials/Brass")));Casing->RegisterComponent();
        Shell->SetActorLocation(IsLocallyControlled()?WeaponRig->GetComponentLocation()+Camera->GetRightVector()*3.f:GetActorLocation()+FVector(0,0,48));
        Shell->SetActorScale3D(FVector(.008f,.008f,.017f));Casing->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);Casing->SetCollisionResponseToChannel(ECC_Pawn,ECR_Ignore);
        Casing->SetSimulatePhysics(true);Casing->AddImpulse(GetActorRightVector()*120.f+FVector(0,0,80),NAME_None,true);
        Casing->AddAngularImpulseInDegrees(FVector(600,300,0),NAME_None,true);Shell->SetLifeSpan(5.f);
    }
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
    GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));GetMesh()->SetSimulatePhysics(true);WeaponRig->SetVisibility(false,true);AimUp();
    if (HasAuthority()) { Handgun->CancelReload(); if (auto* GM=GetWorld()->GetAuthGameMode<ASpychoGameMode>()) GM->OnDeath(this); }
}
void ASpychoCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps); DOREPLIFETIME(ASpychoCharacter,bTestOpponent); DOREPLIFETIME(ASpychoCharacter,bCareful);DOREPLIFETIME(ASpychoCharacter,bSprinting);
    DOREPLIFETIME(ASpychoCharacter,Coins);DOREPLIFETIME(ASpychoCharacter,DuelSlot);
}
