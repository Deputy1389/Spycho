#pragma once
#include "CoreMinimal.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "SpychoSurface.generated.h"

UCLASS(BlueprintType)
class SPYCHO_API USpychoSurface : public UPhysicalMaterial
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Penetration") bool bPenetrable = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Penetration", meta=(ClampMin="0")) float ResistancePerCm = 0.5f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Penetration", meta=(ClampMin="0")) float EntryCost = 3.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Penetration", meta=(ClampMin="0")) float MaxThickness = 60.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audio", meta=(ClampMin="0")) float FootstepGain = 1.f;
};
