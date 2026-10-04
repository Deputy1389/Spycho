#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SpychoMenu.generated.h"
class USlider;
class UTextBlock;
class UEditableTextBox;
UCLASS()
class SPYCHO_API USpychoMenu : public UUserWidget
{
    GENERATED_BODY()
protected:
    virtual void NativeOnInitialized() override;
    virtual FReply NativeOnKeyDown(const FGeometry& Geometry,const FKeyEvent& Event) override;
private:
    UPROPERTY() TObjectPtr<UEditableTextBox> Address;
    UPROPERTY() TArray<TObjectPtr<UTextBlock>> Values;
    UFUNCTION() void Play();
    UFUNCTION() void Rematch();
    UFUNCTION() void Host();
    UFUNCTION() void Join();
    UFUNCTION() void Quit();
    UFUNCTION() void LeftSound();
    UFUNCTION() void RightSound();
    UFUNCTION() void Sensitivity(float Value);
    UFUNCTION() void Master(float Value);
    UFUNCTION() void Effects(float Value);
    UFUNCTION() void Ambient(float Value);
    UFUNCTION() void Brightness(float Value);
    UFUNCTION() void Difficulty(FString Selection,ESelectInfo::Type Type);
    void UpdateLabels();
};
