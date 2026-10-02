#include "SpychoDoor.h"
#include "SpychoGameState.h"
#include "SpychoCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"
ASpychoDoor::ASpychoDoor()
{
    bReplicates=true; bAlwaysRelevant=true; PrimaryActorTick.bCanEverTick=true;
    RootComponent=CreateDefaultSubobject<USceneComponent>(TEXT("Hinge"));
    Swing=CreateDefaultSubobject<USceneComponent>(TEXT("Swing")); Swing->SetupAttachment(RootComponent);
    Panel=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Panel")); Panel->SetupAttachment(Swing);
    ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube")); Panel->SetStaticMesh(Cube.Object);
    ConstructorHelpers::FObjectFinder<UMaterialInterface> Mat(TEXT("/Game/Materials/Wood.Wood")); Panel->SetMaterial(0,Mat.Object);
    ConstructorHelpers::FObjectFinder<UPhysicalMaterial> Phys(TEXT("/Game/Surfaces/Wood.Wood")); DoorSurface=Phys.Object;
    Panel->SetRelativeLocation(FVector(0,45,102)); Panel->SetRelativeScale3D(FVector(0.045f,0.9f,2.04f));
    Panel->SetCollisionProfileName(TEXT("BlockAllDynamic"));
}
void ASpychoDoor::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform); if (DoorSurface) Panel->SetPhysMaterialOverride(DoorSurface);
}
void ASpychoDoor::Toggle(bool Carefully,AActor* Operator)
{
    if (!HasAuthority()) return;
    if (bOpen)
    {
        // Avoid closing through a player in the doorway.
        for (TActorIterator<ASpychoCharacter> It(GetWorld()); It; ++It)
        {
            FVector Local=GetActorTransform().InverseTransformPosition(It->GetActorLocation());
            if (It->Health && FMath::Abs(Local.X)<45.f && Local.Y>-30.f && Local.Y<120.f) return;
        }
    }
    bOpen=!bOpen; bCareful=Carefully; ForceNetUpdate();
    if (auto* GS=GetWorld()->GetGameState<ASpychoGameState>())
    {
        float Gain=Carefully?.12f:.6f;GS->Noise(ESpychoNoise::Door,Panel->GetComponentLocation(),Gain);GS->NotifyHearing(ESpychoNoise::Door,Panel->GetComponentLocation(),Gain,Operator?Operator:this);
    }
}
void ASpychoDoor::Reset() { bOpen=false; Swing->SetRelativeRotation(FRotator::ZeroRotator); ForceNetUpdate(); }
void ASpychoDoor::Tick(float Dt)
{
    Super::Tick(Dt);
    float Yaw=FMath::FInterpConstantTo(Swing->GetRelativeRotation().Yaw,bOpen?OpenAngle:0.f,Dt,bCareful?55.f:220.f);
    Swing->SetRelativeRotation(FRotator(0,Yaw,0));
}
void ASpychoDoor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps); DOREPLIFETIME(ASpychoDoor,bOpen); DOREPLIFETIME(ASpychoDoor,bCareful);
}
