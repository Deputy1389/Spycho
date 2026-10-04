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
#include "SpychoHunterController.h"
#include "EngineUtils.h"
#include "SpychoAcoustics.h"
#include "Components/AudioComponent.h"
#include "Sound/ReverbEffect.h"

ASpychoGameState::ASpychoGameState()
{
    bReplicates = true;
    PrimaryActorTick.bCanEverTick=true;PrimaryActorTick.TickInterval=.1f;
    HardRoomReverb=CreateDefaultSubobject<UReverbEffect>(TEXT("HardRoomReverb"));
    SoftRoomReverb=CreateDefaultSubobject<UReverbEffect>(TEXT("SoftRoomReverb"));
    HardRoomReverb->DecayTime=.85f;HardRoomReverb->Gain=.3f;HardRoomReverb->ReflectionsGain=.12f;HardRoomReverb->LateGain=.35f;
    SoftRoomReverb->DecayTime=.38f;SoftRoomReverb->Gain=.2f;SoftRoomReverb->ReflectionsGain=.07f;SoftRoomReverb->LateGain=.22f;
    const TCHAR* Paths[] = {TEXT("/Game/Audio/StepWood"), TEXT("/Game/Audio/StepCarpet"), TEXT("/Game/Audio/StepTile"), TEXT("/Game/Audio/Gunshot"), TEXT("/Game/Audio/Reload"), TEXT("/Game/Audio/Door"), TEXT("/Game/Audio/Impact"), TEXT("/Game/Audio/Creak"),TEXT("/Game/Audio/Coin")};
    for (const TCHAR* Path : Paths) { ConstructorHelpers::FObjectFinder<USoundBase> Sound(Path); Sounds.Add(Sound.Object); }
    for (const TCHAR* Name : {TEXT("StepWood"),TEXT("StepCarpet"),TEXT("StepTile")}) for (int32 i=1;i<=3;++i)
    {
        ConstructorHelpers::FObjectFinder<USoundBase> Sound(*FString::Printf(TEXT("/Game/Audio/%s_%d"),Name,i));FootstepVariants.Add(Sound.Object);
    }
    Attenuation = CreateDefaultSubobject<USoundAttenuation>(TEXT("SpatialAudio"));
    auto& S = Attenuation->Attenuation;
    S.bAttenuate = true; S.bSpatialize = true; S.bEnableOcclusion = false;
    S.AttenuationShapeExtents = FVector(100.f); S.FalloffDistance = 2400.f;
    S.bEnableReverbSend=true;S.ReverbSendMethod=EReverbSendMethod::Manual;S.ManualReverbSendLevel=.12f;
    S.OcclusionTraceChannel = ECC_Visibility; S.OcclusionLowPassFilterFrequency = 1800.f;
    S.OcclusionVolumeAttenuation = 0.65f; S.OcclusionInterpolationTime = 0.12f;
}
void ASpychoGameState::Tick(float Dt)
{
    Super::Tick(Dt);if (GetNetMode()==NM_DedicatedServer) return;
    auto* PC=GetWorld()->GetFirstPlayerController();if (!PC||!PC->PlayerCameraManager) return;
    int32 Room=SpychoAcoustics::RoomAt(PC->PlayerCameraManager->GetCameraLocation());
    if (Room==ListenerRoom) return;ListenerRoom=Room;
    if (Room<0) UGameplayStatics::DeactivateReverbEffect(this,TEXT("House"));
    else UGameplayStatics::ActivateReverbEffect(this,Room==0||Room==3||Room==5?SoftRoomReverb:HardRoomReverb,TEXT("House"),1.f,.45f,.3f);
}
bool ASpychoGameState::IsDebug() const
{
    auto* PC = Cast<ASpychoPlayerController>(GetWorld()->GetFirstPlayerController());
    return PC && PC->bDebug;
}
void ASpychoGameState::NotifyHearing(ESpychoNoise Kind,FVector Location,float Gain,AActor* Source)
{
    if (!HasAuthority()) return;
    for (TActorIterator<ASpychoHunterController> It(GetWorld());It;++It) It->HearNoise(Kind,Location,Gain,Source);
}
void ASpychoGameState::Noise_Implementation(ESpychoNoise Kind, FVector Location, float Gain)
{
    if (GetNetMode() == NM_DedicatedServer) return;
    const int32 Index = static_cast<int32>(Kind);
    // One shared acoustic model informs playback and bot hearing. Avoid applying
    // native occlusion a second time, which muffled floor-origin sounds twice.
    FSpychoAcousticPath Path;
    if (Index<=2) Location.Z+=12.f;
    auto* PC=GetWorld()->GetFirstPlayerController();
    if (PC && PC->PlayerCameraManager)
    {
        Path=SpychoAcoustics::Probe(GetWorld(),Location,PC->PlayerCameraManager->GetCameraLocation(),PC->GetPawn());
    }
    auto* Settings=NewObject<USoundAttenuation>(this);
    Settings->Attenuation=Attenuation->Attenuation;
    Settings->Attenuation.FalloffDistance=Kind==ESpychoNoise::Gunshot?6000.f:1800.f;
    Settings->Attenuation.ManualReverbSendLevel=Kind==ESpychoNoise::Gunshot?.22f:.08f;
    USoundBase* Sound=Sounds.IsValidIndex(Index)?Sounds[Index].Get():nullptr;
    if (Index<=2)
    {
        int32 Variant=FMath::RandRange(0,3);
        if (Variant>0 && FootstepVariants.IsValidIndex(Index*3+Variant-1)) Sound=FootstepVariants[Index*3+Variant-1];
    }
    float Volume=Gain*(Kind==ESpychoNoise::Gunshot?.28f:1.f)*Path.Transmission();
    float Pitch=Index<=2?FMath::FRandRange(.98f,1.02f):1.f;
    if (Sound) if (auto* Audio=UGameplayStatics::SpawnSoundAtLocation(this,Sound,Location,FRotator::ZeroRotator,Volume,Pitch,0.f,Settings))
    {
        Audio->SetLowPassFilterEnabled(Path.Walls>0);Audio->SetLowPassFilterFrequency(Path.Cutoff());
    }
    if (IsDebug())
    {
        DrawDebugSphere(GetWorld(), Location, FMath::Clamp(Gain * 200.f, 25.f, 600.f), 16, FColor::Cyan, false, 1.5f);
        DrawDebugString(GetWorld(), Location + FVector(0,0,30), FString::Printf(TEXT("sound %d gain %.2f; walls %d open path %d"), Index, Gain, Path.Walls,Path.bOpenRoute), nullptr, FColor::Cyan, 1.5f);
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
    DOREPLIFETIME(ASpychoGameState,ScoreA);DOREPLIFETIME(ASpychoGameState,ScoreB);DOREPLIFETIME(ASpychoGameState,bMatchOver);
}
