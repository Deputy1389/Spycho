#include "SpychoDistraction.h"
#include "SpychoGameState.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "UObject/ConstructorHelpers.h"
ASpychoDistraction::ASpychoDistraction()
{
    bReplicates=true;SetReplicateMovement(true);SetNetUpdateFrequency(30.f);
    Collision=CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));SetRootComponent(Collision);Collision->InitSphereRadius(2.f);
    Collision->SetCollisionProfileName(TEXT("BlockAllDynamic"));Collision->SetCollisionResponseToChannel(ECC_Pawn,ECR_Ignore);
    // A tiny distraction must never intercept the camera's hitscan ray when Q
    // and fire arrive together. Projectile movement still collides with walls.
    Collision->SetCollisionResponseToChannel(ECC_Visibility,ECR_Ignore);
    Coin=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Coin"));Coin->SetupAttachment(Collision);
    ConstructorHelpers::FObjectFinder<UStaticMesh> Shape(TEXT("/Engine/BasicShapes/Cylinder"));Coin->SetStaticMesh(Shape.Object);
    ConstructorHelpers::FObjectFinder<UMaterialInterface> Metal(TEXT("/Game/Materials/Brass"));Coin->SetMaterial(0,Metal.Object);
    Coin->SetRelativeScale3D(FVector(.032f,.032f,.006f));Coin->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Movement=CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Throw"));Movement->SetUpdatedComponent(Collision);
    Movement->bShouldBounce=true;Movement->Bounciness=.42f;Movement->Friction=.38f;Movement->BounceVelocityStopSimulatingThreshold=65.f;
    Movement->ProjectileGravityScale=1.f;Movement->bRotationFollowsVelocity=true;Movement->bForceSubStepping=true;
    Movement->OnProjectileBounce.AddDynamic(this,&ASpychoDistraction::Bounce);
}
void ASpychoDistraction::BeginPlay()
{
    Super::BeginPlay();SetLifeSpan(60.f);
    if (!HasAuthority()) Movement->Deactivate();
}
void ASpychoDistraction::Launch(FVector Velocity) { Movement->Velocity=Velocity; }
void ASpychoDistraction::Bounce(const FHitResult& Hit,const FVector& ImpactVelocity)
{
    if (!HasAuthority() || AudibleBounces>=3 || GetWorld()->GetTimeSeconds()-LastSound<.18f || ImpactVelocity.Size()<80.f) return;
    ++AudibleBounces;LastSound=GetWorld()->GetTimeSeconds();
    LastLanding=Hit.ImpactPoint;
    if (AudibleBounces==1) FirstLanding=Hit.ImpactPoint;
    if (auto* GS=GetWorld()->GetGameState<ASpychoGameState>())
    {
        float Gain=.5f/FMath::Sqrt(float(AudibleBounces));
        GS->Noise(ESpychoNoise::Distraction,Hit.ImpactPoint+Hit.ImpactNormal*8.f,Gain);
        // The bot hears the landing, never the thrower's location.
        GS->NotifyHearing(ESpychoNoise::Distraction,Hit.ImpactPoint,Gain,this);
    }
}
