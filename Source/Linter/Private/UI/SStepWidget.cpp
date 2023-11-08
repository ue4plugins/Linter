// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
#include "UI/SStepWidget.h"

#include "LinterStyle.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SThrobber.h"
#include "Widgets/Text/SRichTextBlock.h"
#include "Misc/EngineVersionComparison.h"

#if UE_VERSION_OLDER_THAN(5, 1, 0)
using FAppStyle = FEditorStyle;
#endif

bool SStepWidget::IsStepCompleted(const bool bAllowWarning) const {
    const EStepStatus Status = StepStatus.Get();
    if (bAllowWarning && Status == Warning) {
        return true;
    }

    return Status == Success;
}

void SStepWidget::Construct(const FArguments& Args) {
    const float PaddingAmount = FLinterStyle::Get()->GetFloat("Linter.Padding");

    StepStatus = Args._StepStatus;
    OnPerformAction = Args._OnPerformAction;
    StepActionText = Args._StepActionText;
    ShowStepStatusIcon = Args._ShowStepStatusIcon;

    // Visibility lambda based on whether step is in progress
    auto VisibleIfInProgress = [this]() {
        return StepStatus.Get(NoStatus) == InProgress ? EVisibility::Visible : EVisibility::Collapsed;
    };

    // Enabled lambda based on whether this widget has a step status that requires action
    auto EnabledBasedOnStepStatus = [this]() -> bool {
        switch (StepStatus.Get(NoStatus)) {
            case NoStatus:
            case InProgress:
            case Success: return false;
            case Unknown:
            case Warning:
            case Error:
            case NeedsUpdate: return true;
        }
        return false;
    };

    // clang-format off
    // @formatter:off
    ChildSlot
    [
        SNew(SBorder)
        .BorderImage(FAppStyle::GetBrush("NoBorder"))
        .Padding(PaddingAmount)
        [
            SNew(SBorder)
	    .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
	    .Padding(PaddingAmount)
            [
                SNew(SHorizontalBox)
                // Status Image
                + SHorizontalBox::Slot()
                .Padding(PaddingAmount)
                .AutoWidth()
                [
                    SNew(SImage)
		    .Visibility_Lambda([&]() {
                        return StepStatus.Get(NoStatus) == NoStatus || !ShowStepStatusIcon.Get(true) ? EVisibility::Collapsed : EVisibility::Visible;
                    })
		    .Image_Lambda([&]() {
                        switch (StepStatus.Get(NoStatus)) {
                            case NoStatus:
                            case Unknown: return FLinterStyle::Get()->GetBrush("Linter.Step.Unknown");
                            case InProgress:
                            case NeedsUpdate: return FLinterStyle::Get()->GetBrush("Linter.Step.Working");
                            case Warning: return FLinterStyle::Get()->GetBrush("Linter.Step.Warning");
                            case Error: return FLinterStyle::Get()->GetBrush("Linter.Step.Error");
                            case Success: return FLinterStyle::Get()->GetBrush("Linter.Step.Good");
                        }

                        return FLinterStyle::Get()->GetBrush("Linter.Step.Unknown");
                    })
                ]
                // Template thumbnail image
                + SHorizontalBox::Slot()
                .Padding(4.0)
                .AutoWidth()
                .VAlign(VAlign_Top)
                [
                    SNew(SImage)
		    .Visibility(Args._Icon.IsSet() ? EVisibility::SelfHitTestInvisible : EVisibility::Collapsed)
		    .Image(Args._Icon)
                ]
                // Template name and description
                + SHorizontalBox::Slot()
                [
                    SNew(SVerticalBox)

                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(PaddingAmount)
                    [
                        SNew(STextBlock)
			.Text(Args._StepName)
			.TextStyle(FLinterStyle::Get(), "Linter.Report.RuleTitle")
                    ]

                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(PaddingAmount)
                    [
                        SNew(SRichTextBlock)
			.Text(Args._StepDesc)
			.TextStyle(FLinterStyle::Get(), "Linter.Report.DescriptionText")
			.AutoWrapText(true)
                    ]
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(PaddingAmount)
                    [
                        SNew(SHorizontalBox)
                        + SHorizontalBox::Slot()
                        .AutoWidth()
                        [
                            SNew(SButton)
			    .IsEnabled_Lambda(EnabledBasedOnStepStatus)
			    .Visibility_Lambda([&]() {
                                return StepStatus.Get(NoStatus) == NoStatus ? EVisibility::Collapsed : EVisibility::Visible;
                            })
			    .OnClicked_Lambda([&]() {
                                FScopedSlowTask SlowTask(1.0f, StepActionText.Get(FText()));
                                SlowTask.MakeDialog();

                                OnPerformAction.ExecuteIfBound(SlowTask);
                                return FReply::Handled();
                            })
                            [
                                SNew(STextBlock)
                                .Text(StepActionText)
                            ]
                        ]
                        + SHorizontalBox::Slot()
                        .AutoWidth()
                        [
                            SNew(SThrobber)
                            .Visibility_Lambda(VisibleIfInProgress)
                        ]
                    ]
                ]
            ]
        ]
    ];
    // clang-format off
    // @formatter:off
}
