#include "SpychoDoor.h"
#include "SpychoGameState.h"
#include "SpychoCharacter.h"
#include "SpychoHealth.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"
ASpychoDoor::ASpychoDoor()
{
    bReplicates=true; bAlwaysRelevant=true;SetNetUpdateFrequency(30.f); PrimaryActorTick.bCanEverTick=true;
    RootComponent=CreateDefaultSubobject<USceneComponent>(TEXT("Hinge"));
    Swing=CreateDefaultSubobject<USceneComponent>(TEXT("Swing")); Swing->SetupAttachment(RootComponent);
    Panel=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Panel")); Panel->SetupAttachment(Swing);
    ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube")); Panel->SetStaticMesh(Cube.Object);
    ConstructorHelpers::FObjectFinder<UMaterialInterface> Mat(TEXT("/Game/Materials/Wood.Wood")); Panel->SetMaterial(0,Mat.Object);
    ConstructorHelpers::FObjectFinder<UPhysicalMaterial> Phys(TEXT("/Game/Surfaces/Wood.Wood")); DoorSurface=Phys.Object;
    Panel->SetRelativeLocation(FVector(0,50.5f,105.25f)); Panel->SetRelativeScale3D(FVector(0.045f,1.01f,2.105f));
    Panel->SetCollisionProfileName(TEXT("BlockAllDynamic"));
    ConstructorHelpers::FObjectFinder<UMaterialInterface> Brass(TEXT("/Game/Materials/Brass"));
    HandleFront=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HandleFront"));HandleFront->SetupAttachment(Swing);
    HandleBack=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HandleBack"));HandleBack->SetupAttachment(Swing);
    for (auto* Handle : {HandleFront.Get(),HandleBack.Get()})
    {
        Handle->SetStaticMesh(Cube.Object);Handle->SetMaterial(0,Brass.Object);Handle->SetRelativeScale3D(FVector(.07f,.14f,.025f));
        Handle->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    HandleFront->SetRelativeLocation(FVector(-5,89,98));HandleBack->SetRelativeLocation(FVector(5,89,98));
    Inset=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Inset"));Inset->SetupAttachment(Swing);Inset->SetStaticMesh(Cube.Object);
    Inset->SetMaterial(0,Mat.Object);Inset->SetRelativeLocation(FVector(0,50.5f,146));Inset->SetRelativeScale3D(FVector(.047f,.73f,.84f));
    Inset->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Inset->SetVisibility(false);
    // Shallow raised panels and stepped mouldings follow the moving leaf.
    // They never change its sealed collision shape or penetration behavior.
    ConstructorHelpers::FObjectFinder<UMaterialInterface> Paint(TEXT("/Game/Materials/ReferenceDoor"));
    int32 DetailIndex=0;
    auto Detail=[&](const FVector& Location,const FVector& Size)
    {
        auto* Part=CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("PanelDetail%d"),DetailIndex++));
        Part->SetupAttachment(Swing);Part->SetStaticMesh(Cube.Object);Part->SetMaterial(0,Paint.Succeeded()?Paint.Object:Mat.Object);
        Part->SetRelativeLocation(Location);Part->SetRelativeScale3D(Size/100.f);Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        PanelDetails.Add(Part);
    };
    for (float Side : {-1.f,1.f}) for (const FVector2D& Section : {FVector2D(159,68),FVector2D(94,42),FVector2D(37,40)})
    {
        Detail(FVector(Side*2.4f,50.5f,Section.X),FVector(.4f,76,Section.Y));
        for (float Sign : {-1.f,1.f})
        {
            Detail(FVector(Side*2.8f,50.5f+Sign*39.f,Section.X),FVector(1.2f,3,Section.Y+4));
            Detail(FVector(Side*2.8f,50.5f,Section.X+Sign*(Section.Y/2+1)),FVector(1.2f,81,3));
            Detail(FVector(Side*3.3f,50.5f+Sign*37.f,Section.X),FVector(.5f,1,Section.Y));
            Detail(FVector(Side*3.3f,50.5f,Section.X+Sign*(Section.Y/2-1)),FVector(.5f,75,1));
        }
    }
}
void ASpychoDoor::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform); if (DoorSurface) Panel->SetPhysMaterialOverride(DoorSurface);
}
void ASpychoDoor::Toggle(bool Carefully,AActor* Operator)
{
    if (!HasAuthority()) return;
    if (bOpen && WouldOverlapPawn(0.f)) return;
    if (!bOpen)
    {
        float Side=Operator?GetActorTransform().InverseTransformPosition(Operator->GetActorLocation()).X:OpenAngle;
        SwingAngle=FMath::Abs(OpenAngle)*(Side>=0?1.f:-1.f);
    }
    bOpen=!bOpen; bCareful=Carefully; ForceNetUpdate();
    if (auto* GS=GetWorld()->GetGameState<ASpychoGameState>())
    {
        float Gain=Carefully?.12f:.6f;GS->Noise(ESpychoNoise::Door,Panel->GetComponentLocation(),Gain);GS->NotifyHearing(ESpychoNoise::Door,Panel->GetComponentLocation(),Gain,Operator?Operator:this);
    }
}
void ASpychoDoor::Reset() { bOpen=false;PanelYaw=0.f; Swing->SetRelativeRotation(FRotator::ZeroRotator); ForceNetUpdate(); }
bool ASpychoDoor::WouldOverlapPawn(float Yaw) const
{
    FTransform Proposed(FRotator(0,GetActorRotation().Yaw+Yaw,0),GetActorLocation());
    for (TActorIterator<ASpychoCharacter> It(GetWorld());It;++It)
    {
        if (It->Health->Health<=0.f) continue;
        FVector P=Proposed.InverseTransformPosition(It->GetActorLocation());
        // Inflate the panel by the character capsule. Pausing instead of pushing
        // avoids a door abruptly launching or trapping the operator.
        if (FMath::Abs(P.X)<34.f && P.Y>-30.f && P.Y<132.f && P.Z>-88.f && P.Z<298.f) return true;
    }
    return false;
}
void ASpychoDoor::Tick(float Dt)
{
    Super::Tick(Dt);
    float Yaw=FMath::FInterpConstantTo(Swing->GetRelativeRotation().Yaw,bOpen?SwingAngle:0.f,Dt,bCareful?55.f:220.f);
    if (HasAuthority()) { if (!WouldOverlapPawn(Yaw)) PanelYaw=Yaw;Swing->SetRelativeRotation(FRotator(0,PanelYaw,0)); }
    else Swing->SetRelativeRotation(FRotator(0,FMath::FInterpTo(Swing->GetRelativeRotation().Yaw,PanelYaw,Dt,22.f),0));
}
void ASpychoDoor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps); DOREPLIFETIME(ASpychoDoor,bOpen); DOREPLIFETIME(ASpychoDoor,bCareful);
    DOREPLIFETIME(ASpychoDoor,SwingAngle);
    DOREPLIFETIME(ASpychoDoor,PanelYaw);
}
