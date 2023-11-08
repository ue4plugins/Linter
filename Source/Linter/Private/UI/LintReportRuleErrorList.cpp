// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.

#include "UI/LintReportRuleErrorList.h"
#include "LinterStyle.h"
#include "Framework/Views/ITypedTableView.h"
#include "LintRule.h"
#include "UI/LintReportRuleError.h"

#define LOCTEXT_NAMESPACE "LintReport"

void SLintReportRuleErrorList::Construct(const FArguments& Args) {
    RuleViolations = Args._RuleViolations;
    const float PaddingAmount = FLinterStyle::Get()->GetFloat("Linter.Padding");

    // clang-format off
    // @formatter:off
    ChildSlot
    [
        SNew(SListView<TSharedPtr<FLintRuleViolation>>)
	.SelectionMode(ESelectionMode::None)
	.ListItemsSource(&RuleViolations.Get())
	.OnGenerateRow_Lambda([&](const TSharedPtr<FLintRuleViolation> InItem, const TSharedRef<STableViewBase>& OwnerTable) {
            return
	        SNew(STableRow<TSharedPtr<FLintRuleViolation>>, OwnerTable)
                [
                    SNew(SLintReportRuleError)
                        .RuleViolation(InItem)
                ];
        })
    ];
    // clang-format on
    // @formatter:on
}

#undef LOCTEXT_NAMESPACE
