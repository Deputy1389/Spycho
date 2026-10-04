#pragma once
#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "SpychoArmsAnim.generated.h"

// One native pose path for the view arms and the opponent, including proper
// mesh-space additive recoil and a closed right-hand grip during reloads.
UCLASS(Transient)
class SPYCHO_API USpychoArmsAnim : public UAnimInstance
{
    GENERATED_BODY()
protected:
    virtual FAnimInstanceProxy* CreateAnimInstanceProxy() override;
    virtual void DestroyAnimInstanceProxy(FAnimInstanceProxy* InProxy) override;
};
