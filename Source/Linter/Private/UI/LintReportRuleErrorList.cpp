// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.

#include "UI/LintReportruleErrorList.h"
#include "LinterStyle.h"
#include "IContentBrowserSingleton.h"
#include "Framework/Views/ITypedTableView.h"
#include "LintRule.h"
#include "UI/LintReportRuleError.h"

#define LOCTEXT_NAMESPACE "LintReport"

void SLintReportRuleErrorList::Construct(const FArguments& Args)
{
	RuleViolations = Args._RuleViolations;
	const float PaddingAmount = FLinterStyle::Get()->GetFloat("Linter.Padding");
	ChildSlot
	[
		SNew(SListView<TSharedPtr<FLintRuleViolation>>)
		.SelectionMode(ESelectionMode::None)
		.ListItemsSource(&RuleViolations.Get())
		.OnGenerateRow_Lambda([&](const TSharedPtr<FLintRuleViolation> InItem, const TSharedRef<STableViewBase>& OwnerTable)
		{
		return SNew(STableRow<TSharedPtr<FLintRuleViolation>>, OwnerTable)
			[
				SNew(SLintReportRuleError)
				.RuleViolation(InItem)
			];
		})
	];
}