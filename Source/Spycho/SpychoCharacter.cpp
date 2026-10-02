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
    Gun=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HandgunView")); Gun->SetupAttachment(Camera); Gun->SetStaticMesh(Cube.Object);
    Gun->SetRelativeLocation(FVector(35,13,-17)); Gun->SetRelativeScale3D(FVector(0.22f,0.045f,0.055f)); Gun->SetMaterial(0,Dark.Object);
    Gun->SetCollisionEnabled(ECollisionEnabled::NoCollision); Gun->SetOnlyOwnerSee(true); Gun->SetCastShadow(false);
    auto* Grip=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Grip")); Grip->SetupAttachment(Gun); Grip->SetStaticMesh(Cube.Object);
    Grip->SetRelativeLocation(FVector(-24,0,-85)); Grip->SetRelativeScale3D(FVector(0.3f,0.9f,2.3f)); Grip->SetMaterial(0,Dark.Object);
    Grip->SetCollisionEnabled(ECollisionEnabled::NoCollision); Grip->SetOnlyOwnerSee(true); Grip->SetCastShadow(false);
}
void ASpychoCharacter::BeginPlay() { Super::BeginPlay(); LastStepPosition=GetActorLocation(); }
void ASpychoCharacter::SetupPlayerInputComponent(UInputComponent* Input)
{
    Super::SetupPlayerInputComponent(Input);
    Input->BindAxis("Forward",this,&ASpychoCharacter::Forward); Input->BindAxis("Right",this,&ASpychoCharacter::Right);
    Input->BindAxis("Turn",this,&ASpychoCharacter::Turn); Input->BindAxis("Look",this,&ASpychoCharacter::Look);
    Input->BindAction("Fire",IE_Pressed,this,&ASpychoCharacter::Fire); Input->BindAction("Reload",IE_Pressed,this,&ASpychoCharacter::Reload);
    Input->BindAction("Interact",IE_Pressed,this,&ASpychoCharacter::Interact);
    Input->BindAction("Crouch",IE_Pressed,this,&ASpychoCharacter::CrouchDown); Input->BindAction("Crouch",IE_Released,this,&ASpychoCharacter::CrouchUp);
    Input->BindAction("Careful",IE_Pressed,this,&ASpychoCharacter::CarefulDown); Input->BindAction("Careful",IE_Released,this,&ASpychoCharacter::CarefulUp);
}
void ASpychoCharacter::Forward(float V) { if (Health->Health>0.f) AddMovementInput(FRotationMatrix(FRotator(0,GetControlRotation().Yaw,0)).GetUnitAxis(EAxis::X),V); }
void ASpychoCharacter::Right(float V) { if (Health->Health>0.f) AddMovementInput(FRotationMatrix(FRotator(0,GetControlRotation().Yaw,0)).GetUnitAxis(EAxis::Y),V); }
void ASpychoCharacter::Turn(float V) { AddControllerYawInput(V*0.65f); }
void ASpychoCharacter::Look(float V) { AddControllerPitchInput(V*0.65f); }
void ASpychoCharacter::Fire() { if (Health->Health>0.f) ServerFire(Camera->GetComponentRotation()); }
void ASpychoCharacter::Reload() { ServerReload(); }
void ASpychoCharacter::Interact() { ServerInteract(); }
void ASpychoCharacter::CrouchDown() { if (Health->Health>0.f) Crouch(); }
void ASpychoCharacter::CrouchUp() { UnCrouch(); }
void ASpychoCharacter::CarefulDown() { bCareful=true; OnRep_Careful(); ServerCareful(true); }
void ASpychoCharacter::CarefulUp() { bCareful=false; OnRep_Careful(); ServerCareful(false); }
void ASpychoCharacter::ServerCareful_Implementation(bool V) { bCareful=V; OnRep_Careful(); }
void ASpychoCharacter::OnRep_Careful() { GetCharacterMovement()->MaxWalkSpeed=bCareful?105.f:210.f; }
void ASpychoCharacter::ServerFire_Implementation(FRotator Aim) { Handgun->Fire(Aim); }
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
    GunKick=FMath::FInterpTo(GunKick,0.f,Dt,12.f); Gun->SetRelativeLocation(FVector(35-GunKick,13,-17));
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
void ASpychoCharacter::ShotFeedback() { GunKick=7.f; if (IsLocallyControlled()) { AddControllerPitchInput(-2.5f); AddControllerYawInput(FMath::FRandRange(-0.5f,0.5f)); } }
void ASpychoCharacter::Die()
{
    if (bDeathHandled) return; bDeathHandled=true;
    GetCharacterMovement()->DisableMovement(); GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Body->SetRelativeRotation(FRotator(80,0,0)); Body->SetRelativeLocation(FVector(0,0,-65)); Gun->SetVisibility(false,true);
    if (HasAuthority()) { Handgun->CancelReload(); if (auto* GM=GetWorld()->GetAuthGameMode<ASpychoGameMode>()) GM->OnDeath(this); }
}
void ASpychoCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps); DOREPLIFETIME(ASpychoCharacter,bTestOpponent); DOREPLIFETIME(ASpychoCharacter,bCareful);
}
