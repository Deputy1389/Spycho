#include "SpychoHealth.h"
#include "SpychoCharacter.h"
#include "Net/UnrealNetwork.h"
USpychoHealth::USpychoHealth() { SetIsReplicatedByDefault(true); }
void USpychoHealth::Damage(float Amount)
{
    if (!GetOwner()->HasAuthority() || Health <= 0.f) return;
    Health = FMath::Max(0.f, Health - FMath::Max(0.f, Amount));
    OnRep_Health();
}
void USpychoHealth::OnRep_Health()
{
    if (Health <= 0.f) if (ASpychoCharacter* C = Cast<ASpychoCharacter>(GetOwner())) C->Die();
}
void USpychoHealth::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps); DOREPLIFETIME(USpychoHealth, Health);
}
