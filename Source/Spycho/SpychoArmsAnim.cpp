#include "SpychoArmsAnim.h"
#include "SpychoCharacter.h"
#include "SpychoHandgun.h"
#include "Animation/AnimInstanceProxy.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimationPoseData.h"
#include "AnimationRuntime.h"
#include "Components/SkeletalMeshComponent.h"

namespace
{
    struct FSpychoPoseProxy : FAnimInstanceProxy
    {
        explicit FSpychoPoseProxy(UAnimInstance* Instance):FAnimInstanceProxy(Instance) {}
        UAnimSequence* Idle=nullptr;
        UAnimSequence* Movement=nullptr;
        UAnimSequence* Reload=nullptr;
        UAnimSequence* Fire=nullptr;
        float Cycle=0,MoveCycle=0,MoveWeight=0,ReloadWeight=0,ReloadTime=0,FireTime=0,FireWeight=0;
        virtual void PreUpdate(UAnimInstance* Instance,float Dt) override
        {
            FAnimInstanceProxy::PreUpdate(Instance,Dt);
            auto* C=Cast<ASpychoCharacter>(Instance->TryGetPawnOwner());if (!C) return;
            Idle=C->IdleAnimation;Reload=C->ReloadAnimation;Fire=C->FireAnimation;
            float Speed=C->GetVelocity().Size2D();bool View=Instance->GetSkelMeshComponent()==C->FirstPersonArms;
            Movement=Speed>300.f?C->RunAnimation:C->WalkAnimation;
            Cycle+=Dt;MoveCycle+=Dt*FMath::Clamp(Speed/(Speed>300.f?420.f:180.f),.2f,1.6f);
            MoveWeight=FMath::FInterpTo(MoveWeight,View?0.f:FMath::Clamp(Speed/40.f,0.f,1.f),Dt,12.f);
            bool Reloading=C->IsReloadingPresentation();
            ReloadWeight=FMath::FInterpTo(ReloadWeight,Reloading?1.f:0.f,Dt,20.f);
            if (Reloading&&Reload) ReloadTime=C->GetReloadProgress()*Reload->GetPlayLength();
            float Shot=C->GetShotProgress();FireWeight=Shot<1.f?(1.f-Shot)*.55f:0.f;
            FireTime=Fire?FMath::Clamp(Shot,0.f,1.f)*Fire->GetPlayLength():0.f;
        }
        static void Sample(UAnimSequence* Sequence,float Time,FPoseContext& Pose)
        {
            Pose.ResetToRefPose();if (!Sequence) return;
            FAnimationPoseData Data(Pose);
            Sequence->GetAnimationPose(Data,FAnimExtractContext(FMath::Clamp(Time,0.f,Sequence->GetPlayLength()),false));
        }
        virtual bool Evaluate(FPoseContext& Output) override
        {
            FPoseContext Base(Output),Other(Output),Grip(Output);
            Sample(Idle,Idle?FMath::Fmod(Cycle,Idle->GetPlayLength()):0.f,Base);
            Grip.Pose.CopyBonesFrom(Base.Pose);
            FAnimationPoseData BaseData(Base),OtherData(Other),OutData(Output);
            if (MoveWeight>.001f&&Movement)
            {
                Sample(Movement,FMath::Fmod(MoveCycle,Movement->GetPlayLength()),Other);
                FAnimationRuntime::BlendTwoPosesTogetherInPlace(BaseData,OtherData,1.f-MoveWeight);
            }
            if (FireWeight>.001f&&Fire)
            {
                Sample(Fire,FireTime,Other);
                FAnimationRuntime::AccumulateMeshSpaceRotationAdditiveToLocalPose(BaseData,OtherData,FireWeight);
            }
            if (ReloadWeight>.001f&&Reload)
            {
                Sample(Reload,ReloadTime,Other);
                FAnimationRuntime::BlendTwoPosesTogether(BaseData,OtherData,1.f-ReloadWeight,OutData);
                // The reload clip opens the firing hand. Retain its gripping
                // finger pose while the support hand handles the magazine.
                const auto& Bones=Output.Pose.GetBoneContainer();
                const auto& ReferenceBones=Bones.GetReferenceSkeleton();
                for (int32 i=0;i<ReferenceBones.GetNum();++i)
                {
                    FString Name=ReferenceBones.GetBoneName(i).ToString();
                    if (!Name.EndsWith(TEXT("_r"))) continue;
                    if (!(Name.StartsWith(TEXT("thumb_"))||Name.StartsWith(TEXT("index_"))||Name.StartsWith(TEXT("middle_"))||Name.StartsWith(TEXT("ring_"))||Name.StartsWith(TEXT("pinky_")))) continue;
                    FCompactPoseBoneIndex Index=Bones.MakeCompactPoseIndex(FMeshPoseBoneIndex(i));
                    if (Index.IsValid()) Output.Pose[Index]=Grip.Pose[Index];
                }
            }
            else
            {
                Output.Pose.CopyBonesFrom(Base.Pose);Output.Curve=Base.Curve;Output.CustomAttributes=Base.CustomAttributes;
            }
            Output.Pose.NormalizeRotations();return true;
        }
    };
}
FAnimInstanceProxy* USpychoArmsAnim::CreateAnimInstanceProxy() { return new FSpychoPoseProxy(this); }
void USpychoArmsAnim::DestroyAnimInstanceProxy(FAnimInstanceProxy* InProxy) { delete InProxy; }
