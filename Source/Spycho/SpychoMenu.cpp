#include "SpychoMenu.h"
#include "SpychoPlayerController.h"
#include "SpychoOptions.h"
#include "SpychoGameState.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Components/ScaleBox.h"
#include "Components/TextBlock.h"
#include "Components/Slider.h"
#include "Components/ComboBoxString.h"
#include "Components/EditableTextBox.h"
#include "Kismet/KismetSystemLibrary.h"

void USpychoMenu::NativeOnInitialized()
{
    Super::NativeOnInitialized();SetIsFocusable(true);
    auto* PC=Cast<ASpychoPlayerController>(GetOwningPlayer());if (!PC||!PC->Options) return;
    auto* O=PC->Options.Get();
    auto* Root=WidgetTree->ConstructWidget<UOverlay>();WidgetTree->RootWidget=Root;
    auto* Back=WidgetTree->ConstructWidget<UBorder>();Back->SetBrushColor(FLinearColor(.008f,.012f,.011f,.96f));
    auto* BackSlot=Root->AddChildToOverlay(Back);BackSlot->SetHorizontalAlignment(HAlign_Fill);BackSlot->SetVerticalAlignment(VAlign_Fill);
    auto* Scale=WidgetTree->ConstructWidget<UScaleBox>();Scale->SetStretch(EStretch::ScaleToFit);
    auto* Center=Root->AddChildToOverlay(Scale);Center->SetHorizontalAlignment(HAlign_Center);Center->SetVerticalAlignment(VAlign_Center);
    auto* Size=WidgetTree->ConstructWidget<USizeBox>();Size->SetWidthOverride(1000);Size->SetHeightOverride(650);Scale->AddChild(Size);
    auto* Columns=WidgetTree->ConstructWidget<UHorizontalBox>();Size->AddChild(Columns);
    auto* Left=WidgetTree->ConstructWidget<UVerticalBox>();auto* Right=WidgetTree->ConstructWidget<UVerticalBox>();
    auto* L=Columns->AddChildToHorizontalBox(Left);L->SetSize(FSlateChildSize(ESlateSizeRule::Fill));L->SetPadding(FMargin(30,12,40,12));
    auto* R=Columns->AddChildToHorizontalBox(Right);R->SetSize(FSlateChildSize(ESlateSizeRule::Fill));R->SetPadding(FMargin(30,12,30,12));
    auto Text=[&](UVerticalBox* Box,const FString& Label,int32 FontSize,FLinearColor Color=FLinearColor(.78f,.8f,.74f))
    {
        auto* T=WidgetTree->ConstructWidget<UTextBlock>();T->SetText(FText::FromString(Label));auto Font=T->GetFont();Font.Size=FontSize;T->SetFont(Font);T->SetColorAndOpacity(Color);T->SetAutoWrapText(true);
        Box->AddChildToVerticalBox(T)->SetPadding(FMargin(0,0,0,10));return T;
    };
    auto Button=[&](UVerticalBox* Box,const FString& Label)
    {
        auto* B=WidgetTree->ConstructWidget<UButton>();B->SetBackgroundColor(FLinearColor(.16f,.2f,.18f));
        auto* T=WidgetTree->ConstructWidget<UTextBlock>();T->SetText(FText::FromString(Label));auto Font=T->GetFont();Font.Size=18;T->SetFont(Font);T->SetColorAndOpacity(FLinearColor(.93f,.9f,.75f));B->AddChild(T);
        auto* Wrap=WidgetTree->ConstructWidget<USizeBox>();Wrap->SetHeightOverride(42);Wrap->AddChild(B);Box->AddChildToVerticalBox(Wrap)->SetPadding(FMargin(0,0,0,10));return B;
    };
    Text(Left,TEXT("SPYCHO"),52,FLinearColor(.88f,.78f,.52f));
    Text(Left,TEXT("Listen. Commit. Disappear."),22);
    Text(Left,TEXT("One house. One opponent. Thin walls.\nListen for footsteps and doors. Shoot through walls, then move before they answer."),17);
    Button(Left,PC->bStartupMenu?TEXT("PLAY SOLO DUEL"):TEXT("RESUME DUEL"))->OnClicked.AddDynamic(this,&USpychoMenu::Play);
    if (auto* GS=GetWorld()->GetGameState<ASpychoGameState>()) if (GS->bMatchOver&&PC->HasAuthority()) Button(Left,TEXT("REMATCH"))->OnClicked.AddDynamic(this,&USpychoMenu::Rematch);
    Text(Left,TEXT("BOT DIFFICULTY — applied next round"),14);
    auto* Choice=WidgetTree->ConstructWidget<UComboBoxString>();for (const TCHAR* Name:{TEXT("Cautious"),TEXT("Standard"),TEXT("Sharp")}) Choice->AddOption(Name);
    Choice->SetSelectedIndex(O->Difficulty);Left->AddChildToVerticalBox(Choice)->SetPadding(FMargin(0,0,0,15));Choice->OnSelectionChanged.AddDynamic(this,&USpychoMenu::Difficulty);
    Button(Left,TEXT("HOST TWO-PLAYER DUEL"))->OnClicked.AddDynamic(this,&USpychoMenu::Host);
    Address=WidgetTree->ConstructWidget<UEditableTextBox>();Address->SetForegroundColor(FLinearColor(.04f,.055f,.045f));Address->SetText(FText::FromString(TEXT("127.0.0.1:7777")));Address->SetHintText(FText::FromString(TEXT("Host address : port")));Left->AddChildToVerticalBox(Address)->SetPadding(FMargin(0,0,0,8));
    Button(Left,TEXT("JOIN ADDRESS"))->OnClicked.AddDynamic(this,&USpychoMenu::Join);
    Button(Left,TEXT("QUIT"))->OnClicked.AddDynamic(this,&USpychoMenu::Quit);
    Text(Right,TEXT("SETTINGS"),28,FLinearColor(.88f,.78f,.52f));
    auto Slider=[&](const FString& Name,float Value,float Min,float Max)
    {
        Values.Add(Text(Right,Name,17));auto* S=WidgetTree->ConstructWidget<USlider>();S->SetMinValue(Min);S->SetMaxValue(Max);S->SetValue(Value);S->SetSliderHandleColor(FLinearColor(.88f,.78f,.52f));Right->AddChildToVerticalBox(S)->SetPadding(FMargin(0,0,0,20));return S;
    };
    Slider(TEXT("Mouse sensitivity"),O->Sensitivity,.25f,2.5f)->OnValueChanged.AddDynamic(this,&USpychoMenu::Sensitivity);
    Slider(TEXT("Master volume"),O->MasterVolume,0,1)->OnValueChanged.AddDynamic(this,&USpychoMenu::Master);
    Slider(TEXT("Action sounds"),O->EffectsVolume,0,1)->OnValueChanged.AddDynamic(this,&USpychoMenu::Effects);
    Slider(TEXT("House ambience"),O->AmbientVolume,0,1)->OnValueChanged.AddDynamic(this,&USpychoMenu::Ambient);
    Slider(TEXT("Brightness"),O->Brightness,-.5f,1)->OnValueChanged.AddDynamic(this,&USpychoMenu::Brightness);
    Text(Right,TEXT("HEADPHONE CHECK"),14);
    Button(Right,TEXT("PLAY LEFT"))->OnClicked.AddDynamic(this,&USpychoMenu::LeftSound);
    Button(Right,TEXT("PLAY RIGHT"))->OnClicked.AddDynamic(this,&USpychoMenu::RightSound);
    Text(Right,PC->GetNetMode()==NM_Standalone?TEXT("Escape to return • Duel paused"):TEXT("Escape to return • Online duel continues"),14);
    Text(Right,TEXT("WASD move • Shift sprint • Ctrl slow walk\nLMB fire • RMB aim • E door • R reload\nQ coin • Ctrl + E quiet door • Enter rematch"),14);
    UpdateLabels();
}
FReply USpychoMenu::NativeOnKeyDown(const FGeometry& G,const FKeyEvent& E)
{
    if (E.GetKey()==EKeys::Escape) { Play();return FReply::Handled(); }return Super::NativeOnKeyDown(G,E);
}
void USpychoMenu::Play(){if (auto* P=Cast<ASpychoPlayerController>(GetOwningPlayer())) P->StartSolo();}
void USpychoMenu::Rematch(){if (auto* P=Cast<ASpychoPlayerController>(GetOwningPlayer())) { P->CloseMenu();P->ServerRestart(); }}
void USpychoMenu::Host(){if (auto* P=Cast<ASpychoPlayerController>(GetOwningPlayer())) P->Host();}
void USpychoMenu::Join(){if (auto* P=Cast<ASpychoPlayerController>(GetOwningPlayer())) P->Join(Address->GetText().ToString());}
void USpychoMenu::Quit(){if (auto* P=Cast<ASpychoPlayerController>(GetOwningPlayer())) P->SaveOptions();UKismetSystemLibrary::QuitGame(this,GetOwningPlayer(),EQuitPreference::Quit,false);}
void USpychoMenu::LeftSound(){if (auto* P=Cast<ASpychoPlayerController>(GetOwningPlayer())) P->TestSound(false);}
void USpychoMenu::RightSound(){if (auto* P=Cast<ASpychoPlayerController>(GetOwningPlayer())) P->TestSound(true);}
void USpychoMenu::Sensitivity(float V){auto* P=Cast<ASpychoPlayerController>(GetOwningPlayer());P->Options->Sensitivity=V;P->ApplyOptions();UpdateLabels();}
void USpychoMenu::Master(float V){auto* P=Cast<ASpychoPlayerController>(GetOwningPlayer());P->Options->MasterVolume=V;P->ApplyOptions();UpdateLabels();}
void USpychoMenu::Effects(float V){auto* P=Cast<ASpychoPlayerController>(GetOwningPlayer());P->Options->EffectsVolume=V;P->ApplyOptions();UpdateLabels();}
void USpychoMenu::Ambient(float V){auto* P=Cast<ASpychoPlayerController>(GetOwningPlayer());P->Options->AmbientVolume=V;P->ApplyOptions();UpdateLabels();}
void USpychoMenu::Brightness(float V){auto* P=Cast<ASpychoPlayerController>(GetOwningPlayer());P->Options->Brightness=V;P->ApplyOptions();UpdateLabels();}
void USpychoMenu::Difficulty(FString S,ESelectInfo::Type){auto* P=Cast<ASpychoPlayerController>(GetOwningPlayer());P->Options->Difficulty=S==TEXT("Cautious")?0:S==TEXT("Sharp")?2:1;P->ApplyOptions();}
void USpychoMenu::UpdateLabels()
{
    auto* O=Cast<ASpychoPlayerController>(GetOwningPlayer())->Options.Get();
    const FString Labels[]={FString::Printf(TEXT("Mouse sensitivity   %.2f"),O->Sensitivity),FString::Printf(TEXT("Master volume   %.0f%%"),O->MasterVolume*100),FString::Printf(TEXT("Action sounds   %.0f%%"),O->EffectsVolume*100),FString::Printf(TEXT("House ambience   %.0f%%"),O->AmbientVolume*100),FString::Printf(TEXT("Brightness   %+.1f"),O->Brightness)};
    for(int32 I=0;I<Values.Num();++I) Values[I]->SetText(FText::FromString(Labels[I]));
}
