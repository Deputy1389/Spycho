#include "SpychoGameState.h"
#include "SpychoPlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Sound/SoundAttenuation.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "DrawDebugHelpers.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/PlayerCameraManager.h"

ASpychoGameState::ASpychoGameState()
{
    bReplicates = true;
    const TCHAR* Paths[] = {TEXT("/Game/Audio/StepWood"), TEXT("/Game/Audio/StepCarpet"), TEXT("/Game/Audio/StepTile"), TEXT("/Game/Audio/Gunshot"), TEXT("/Game/Audio/Reload"), TEXT("/Game/Audio/Door"), TEXT("/Game/Audio/Impact"), TEXT("/Game/Audio/Creak")};
    for (const TCHAR* Path : Paths) { ConstructorHelpers::FObjectFinder<USoundBase> Sound(Path); Sounds.Add(Sound.Object); }
    Attenuation = CreateDefaultSubobject<USoundAttenuation>(TEXT("SpatialAudio"));
    auto& S = Attenuation->Attenuation;
    S.bAttenuate = true; S.bSpatialize = true; S.bEnableOcclusion = true;
    S.AttenuationShapeExtents = FVector(100.f); S.FalloffDistance = 2400.f;
    S.OcclusionTraceChannel = ECC_Visibility; S.OcclusionLowPassFilterFrequency = 1800.f;
    S.OcclusionVolumeAttenuation = 0.65f; S.OcclusionInterpolationTime = 0.12f;
}
bool ASpychoGameState::IsDebug() const
{
    auto* PC = Cast<ASpychoPlayerController>(GetWorld()->GetFirstPlayerController());
    return PC && PC->bDebug;
}
void ASpychoGameState::Noise_Implementation(ESpychoNoise Kind, FVector Location, float Gain)
{
    if (GetNetMode() == NM_DedicatedServer) return;
    const int32 Index = static_cast<int32>(Kind);
    // Native occlusion handles the closest wall. Count further obstructions to
    // preserve a useful distinction between one partition and several rooms.
    int32 Walls=0;
    auto* PC=GetWorld()->GetFirstPlayerController();
    if (PC && PC->PlayerCameraManager)
    {
        FVector Listener=PC->PlayerCameraManager->GetCameraLocation();
        FCollisionQueryParams Q(SCENE_QUERY_STAT(SpychoAcoustics),false,PC->GetPawn());
        FVector Cursor=Location;
        for (int32 i=0;i<4;++i)
        {
            FHitResult H;
            if (!GetWorld()->LineTraceSingleByChannel(H,Cursor,Listener,ECC_Visibility,Q)) break;
            if (H.GetActor()) Q.AddIgnoredActor(H.GetActor()); else break;
            // Sounds originate on floors or in pawns; those aren't intervening walls.
            if (FVector::DistSquared(H.ImpactPoint,Location)>20.f*20.f) ++Walls;
            Cursor=H.ImpactPoint+(Listener-H.ImpactPoint).GetSafeNormal()*2.f;
        }
    }
    auto* Settings=NewObject<USoundAttenuation>(this);
    Settings->Attenuation=Attenuation->Attenuation;
    Settings->Attenuation.FalloffDistance=Kind==ESpychoNoise::Gunshot?6000.f:1800.f;
    Settings->Attenuation.OcclusionVolumeAttenuation=0.65f*FMath::Pow(0.85f,FMath::Max(0,Walls-1));
    Settings->Attenuation.OcclusionLowPassFilterFrequency=1800.f/FMath::Max(1,Walls);
    if (Sounds.IsValidIndex(Index) && Sounds[Index]) UGameplayStatics::PlaySoundAtLocation(this, Sounds[Index], Location, Gain, FMath::FRandRange(0.96f, 1.04f), 0.f, Settings);
    if (IsDebug())
    {
        DrawDebugSphere(GetWorld(), Location, FMath::Clamp(Gain * 200.f, 25.f, 600.f), 16, FColor::Cyan, false, 1.5f);
        DrawDebugString(GetWorld(), Location + FVector(0,0,30), FString::Printf(TEXT("sound %d gain %.2f; walls %d"), Index, Gain, Walls), nullptr, FColor::Cyan, 1.5f);
    }
}
void ASpychoGameState::Impact_Implementation(FVector Location, FVector Normal, bool bExit)
{
    if (GetNetMode() == NM_DedicatedServer) return;
    AActor* Mark = GetWorld()->SpawnActor<AActor>();
    auto* Mesh = NewObject<UStaticMeshComponent>(Mark);
    Mesh->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder")));
    Mesh->SetMaterial(0, LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/BulletHole.BulletHole")));
    Mark->SetRootComponent(Mesh); Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); Mesh->RegisterComponent();
    Mark->SetActorLocation(Location + Normal * 0.16f);
    Mark->SetActorRotation(FRotationMatrix::MakeFromZ(Normal).Rotator());
    Mark->SetActorScale3D(FVector(bExit ? 0.065f : 0.045f, bExit ? 0.065f : 0.045f, 0.003f));
    Evidence.Add(Mark);
    // Brief dust chips. Evidence remains, collision and wall integrity remain intact.
    for (int32 i=0; i<5; ++i)
    {
        AActor* Dust = GetWorld()->SpawnActor<AActor>();
        auto* Chip = NewObject<UStaticMeshComponent>(Dust);
        Dust->SetRootComponent(Chip); Chip->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")));
        Chip->SetMaterial(0, LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/Drywall.Drywall")));
        Chip->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly); Chip->RegisterComponent();
        Dust->SetActorLocation(Location + Normal * 3.f + FMath::VRand() * 3.f); Dust->SetActorScale3D(FVector(0.015f));
        Chip->SetSimulatePhysics(true); Chip->AddImpulse((Normal + FMath::VRand() * 0.8f) * 0.5f);
        Dust->SetLifeSpan(0.6f);
    }
}
void ASpychoGameState::ShotDebug_Implementation(FVector Start, FVector End, const FString& Detail)
{
    if (IsDebug())
    {
        DrawDebugLine(GetWorld(), Start, End, FColor::Orange, false, 5.f, 0, 1.f);
        DrawDebugPoint(GetWorld(), Start, 6.f, FColor::Green, false, 5.f);
        DrawDebugPoint(GetWorld(), End, 8.f, FColor::Red, false, 5.f);
        DrawDebugString(GetWorld(), End+FVector(0,0,10),Detail,nullptr,FColor::Orange,5.f);
        LastShotDebug = Detail;
    }
}
void ASpychoGameState::OnRep_Round()
{
    for (AActor* Mark : Evidence) if (IsValid(Mark)) Mark->Destroy();
    Evidence.Empty(); LastShotDebug.Empty();
}
void ASpychoGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ASpychoGameState, Round); DOREPLIFETIME(ASpychoGameState, bRoundActive); DOREPLIFETIME(ASpychoGameState, RoundMessage);
}
