#include "Misc/AutomationTest.h"
#include "SpychoRules.h"
#include "SpychoAcoustics.h"

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSpychoPenetrationRules,"Spycho.Rules.Penetration",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FSpychoPenetrationRules::RunTest(const FString&)
{
    TestEqual(TEXT("12cm drywall preserves lethal energy"),SpychoRules::RemainingEnergy(100,12,.45f,3),91.6f);
    TestTrue(TEXT("drywall hit lethal"),SpychoRules::Damage(91.6f,100,130)>100);
    TestTrue(TEXT("wood costs more than drywall"),SpychoRules::RemainingEnergy(100,12,1.8f,7)<91.6f);
    TestEqual(TEXT("masonry exhausts energy"),SpychoRules::RemainingEnergy(100,24,20,100),0.f);
    TestEqual(TEXT("no negative damage"),SpychoRules::Damage(-10,100,130),0.f);
    TestEqual(TEXT("invalid initial energy is safe"),SpychoRules::Damage(10,0,130),0.f);
    float ObliqueThickness=12.f/FMath::Cos(FMath::DegreesToRadians(60.f));
    TestTrue(TEXT("oblique ray expends more energy"),SpychoRules::RemainingEnergy(100,ObliqueThickness,.45f,3)<91.6f);
    float E=SpychoRules::RemainingEnergy(100,12,.45f,3); E=SpychoRules::RemainingEnergy(E,12,.45f,3);
    TestTrue(TEXT("successive surfaces reduce damage"),SpychoRules::Damage(E,100,130)<SpychoRules::Damage(91.6f,100,130));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSpychoAmmoRules,"Spycho.Rules.AmmoAndRound",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FSpychoAmmoRules::RunTest(const FString&)
{
    int32 M=2,R=3; TestEqual(TEXT("partial reserve"),SpychoRules::Reload(M,R,6),3); TestEqual(TEXT("magazine"),M,5);TestEqual(TEXT("reserve"),R,0);
    M=6;R=12;TestEqual(TEXT("full mag does not waste reserve"),SpychoRules::Reload(M,R,6),0);TestEqual(TEXT("reserve conserved"),R,12);
    TestTrue(TEXT("can fire active and alive"),SpychoRules::CanFire(1,false,true,true));
    TestFalse(TEXT("empty"),SpychoRules::CanFire(0,false,true,true));
    TestFalse(TEXT("reload locks fire"),SpychoRules::CanFire(6,true,true,true));
    TestFalse(TEXT("death locks fire"),SpychoRules::CanFire(6,false,false,true));
    TestFalse(TEXT("round end locks fire"),SpychoRules::CanFire(6,false,true,false));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSpychoSoundRules,"Spycho.Rules.AcousticClues",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FSpychoSoundRules::RunTest(const FString&)
{
    FSpychoAcousticPath Same,Thin,Multiple,Closed,Open;
    Thin.Walls=1;Multiple.Walls=3;Closed.Walls=1;Closed.bClosedDoor=true;Open.Walls=1;Open.bOpenRoute=true;
    TestTrue(TEXT("one wall quieter than same room"),Thin.Transmission()<Same.Transmission());
    TestTrue(TEXT("several walls quieter and more muffled"),Multiple.Transmission()<Thin.Transmission()&&Multiple.Cutoff()<Thin.Cutoff());
    TestTrue(TEXT("open route preserves more audible detail"),Open.Transmission()>Closed.Transmission()&&Open.Cutoff()>Closed.Cutoff());
    TestEqual(TEXT("outside not a room"),SpychoAcoustics::RoomAt(FVector(900,0,0)),-1);
    return true;
}
#endif
