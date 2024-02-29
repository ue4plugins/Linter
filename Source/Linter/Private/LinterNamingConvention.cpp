#include "LinterNamingConvention.h"

#include "AnyObject_LinterDummyClass.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "IDetailChildrenBuilder.h"
#include "PropertyCustomizationHelpers.h"
#include "Misc/EngineVersionComparison.h"
#include "Templates/SharedPointer.h"
#if UE_VERSION_NEWER_THAN(5, 0, 0)
#include "UObject/ObjectSaveContext.h"
#endif

TSharedRef<IDetailCustomization> FLinterNamingConventionDetails::MakeInstance() {
    return MakeShareable(new FLinterNamingConventionDetails());
}

void FLinterNamingConventionDetails::CustomizeDetails(class IDetailLayoutBuilder& DetailBuilder) {
    // Edit the Conventions category
    IDetailCategoryBuilder& DetailCategory = DetailBuilder.EditCategory("Conventions");
    const TSharedRef<IPropertyHandle> NamingConventionsProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULinterNamingConvention, ClassNamingConventions), ULinterNamingConvention::StaticClass());

    const TSharedRef<FDetailArrayBuilder> ConventionsPropertyBuilder = MakeShareable(new FDetailArrayBuilder(NamingConventionsProperty));
    ConventionsPropertyBuilder->OnGenerateArrayElementWidget(FOnGenerateArrayElementWidget::CreateSP(this, &FLinterNamingConventionDetails::OnGenerateElementForDetails, &DetailBuilder));


    DetailCategory.AddCustomBuilder(ConventionsPropertyBuilder);
}

void FLinterNamingConventionDetails::OnGenerateElementForDetails(const TSharedRef<IPropertyHandle> StructProperty, int32 ElementIndex, IDetailChildrenBuilder& ChildrenBuilder, IDetailLayoutBuilder* DetailLayout) {
    const TSharedRef<SWidget> RemoveButton = PropertyCustomizationHelpers::MakeRemoveButton(FSimpleDelegate::CreateLambda([this, DetailLayout, ElementIndex] {
            const TSharedRef<IPropertyHandle> NamingConventionsProperty
                = DetailLayout->GetProperty(GET_MEMBER_NAME_CHECKED(ULinterNamingConvention, ClassNamingConventions), ULinterNamingConvention::StaticClass());
            const TSharedPtr<IPropertyHandleArray> NamingConventionsPropertyHandle = NamingConventionsProperty->AsArray();
            NamingConventionsPropertyHandle->DeleteItem(ElementIndex);
        }
        ));

    // clang-format off
    // @formatter:off
    ChildrenBuilder.AddCustomRow(FText::GetEmpty())
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot()
        .AutoWidth()
        [
            RemoveButton
        ]
        + SHorizontalBox::Slot()
        .FillWidth(1.0f)
        [
            SNew(SProperty, StructProperty->GetChildHandle("SoftClassPtr"))
            .ShouldDisplayName(false)
        ]
        + SHorizontalBox::Slot()
        .FillWidth(0.25f)
        [
            SNew(SProperty, StructProperty->GetChildHandle("Variant"))
        ]
        + SHorizontalBox::Slot()
        .FillWidth(0.25f)
        [
            SNew(SProperty, StructProperty->GetChildHandle("Prefix"))
        ]
        + SHorizontalBox::Slot()
        .FillWidth(0.25f)
        [
            SNew(SProperty, StructProperty->GetChildHandle("Suffix"))
        ]
    ];
    // clang-format on
    // @formatter:on
}

ULinterNamingConvention::ULinterNamingConvention(const FObjectInitializer& ObjectInitializer) :
    Super(ObjectInitializer) {
    ClassNamingConventions = TArray<FLinterNamingConventionInfo>();
}

TArray<FLinterNamingConventionInfo> ULinterNamingConvention::GetNamingConventionsForClassVariant(const TSoftClassPtr<UObject> Class, FName Variant /*= NAME_None*/) const {
    TArray<FLinterNamingConventionInfo> NamingConventionList;

    UClass* SearchClass = Class.Get();
    while (NamingConventionList.Num() == 0 && SearchClass != nullptr) {
        NamingConventionList = ClassNamingConventions.FilterByPredicate([SearchClass, Variant](const FLinterNamingConventionInfo& Info) {
            return (Info.SoftClassPtr.Get() == SearchClass && Info.Variant == Variant);
        });

        // Abort if we try to go above UObject
        if (SearchClass == UObject::StaticClass()) {
            break;
        }

        // @HACK: Editor UI won't allow us to select the UObject class in some cases
        if (SearchClass == UAnyObject_LinterDummyClass::StaticClass()) {
            SearchClass = UObject::StaticClass();
            continue;
        }

        // Load our parent class in case we failed to get naming conventions
        SearchClass = SearchClass->GetSuperClass();
    }

    return NamingConventionList;
}

void ULinterNamingConvention::SortConventions() {
    ClassNamingConventions.Sort([](const FLinterNamingConventionInfo& A, const FLinterNamingConventionInfo& B) {
        if (A.SoftClassPtr.GetAssetName() < B.SoftClassPtr.GetAssetName()) {
            return true;
        }

        if (A.SoftClassPtr.GetAssetName() == B.SoftClassPtr.GetAssetName()) {
            int32 Sort = A.Variant.Compare(B.Variant);
            if (Sort < 0) {
                return true;
            }

            if (Sort == 0) {
                Sort = A.Prefix.Compare(B.Prefix);
                if (Sort < 0) {
                    return true;
                }

                if (Sort == 0) {
                    Sort = A.Suffix.Compare(B.Suffix);
                    if (Sort <= 0) {
                        return true;
                    }
                    return false;
                }

                return false;
            }

            return false;
        }

        return false;
    });
}

#if UE_VERSION_NEWER_THAN(5, 0, 0)
void ULinterNamingConvention::PreSave(FObjectPreSaveContext ObjectSaveContext) {
    Super::PreSave(ObjectSaveContext);

    SortConventions();
}
#else
void ULinterNamingConvention::PreSave(const class ITargetPlatform* TargetPlatform) {
    Super::PreSave(TargetPlatform);

    SortConventions();
}
#endif
