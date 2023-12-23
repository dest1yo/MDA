// Copyright 2023 dest1yo. All Rights Reserved.

#include "AnimGraphNode_MDA.h"
#include "MDACommands.h"
#include "MDAEditor.h"
#include "ScopedTransaction.h"
#include "ToolMenus.h"
#include "Kismet2/BlueprintEditorUtils.h"

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(AnimGraphNode_MDA)
#endif

/////////////////////////////////////////////////////
// UAnimGraphNode_MDA

#define LOCTEXT_NAMESPACE "AnimGraphNode_MDA"

UAnimGraphNode_MDA::UAnimGraphNode_MDA(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer), RemovedPinArrayIndex(INDEX_NONE)
{
}

FString UAnimGraphNode_MDA::GetNodeCategory() const
{
	return TEXT("Blends");
}

FLinearColor UAnimGraphNode_MDA::GetNodeTitleColor() const
{
	return FLinearColor(67/255.0f, 142/255.0f, 255/255.0f);
}

// FLinearColor UAnimGraphNode_MDA::GetNodeBodyTintColor() const
// {
// 	return FLinearColor(255.0f, 0.0f, 0.0f);
// }

FText UAnimGraphNode_MDA::GetTooltipText() const
{
	return LOCTEXT("MDATooltip", "Blend multiple poses together by Alpha");
}

FText UAnimGraphNode_MDA::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("Blend", "MDA");
}

void UAnimGraphNode_MDA::GetNodeContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const
{
	// Any non-debugging commands should be disabled
	if (Context->bIsDebugging)
	{
		// TODO: Log
		return;
	}

	// Add a section with MDA label
	FToolMenuSection& Section = Menu->AddSection("AnimGraphNodeMDA", LOCTEXT("MDA", "MDA"));

	// Get the MDA CommandList from MDA EditorModule
	const TSharedPtr<FUICommandList> MDACommandList = FMDAEditorModule::Get().GetMDACommandList();

	// Show a remove pin option on the pin menu
	if (Context->Pin)
	{
		// Debug
		// UE_LOG(LogTemp, Warning, TEXT("%s"), *Context->Pin->PinName.ToString());

		// Only offer the option on input pin
		if (Context->Pin->Direction != EGPD_Input)
		{
			// TODO: Log
			return;
		}

		// Only offer the option on arrayed pins
		// TODO: Whether the method can be optimized
		FProperty* AssociatedProperty;
		int32 ArrayIndex;
		GetPinAssociatedProperty(GetFNodeType(), Context->Pin, /*out*/ AssociatedProperty, /*out*/ ArrayIndex);
		if (ArrayIndex == INDEX_NONE)
		{
			// TODO: Log
			return;
		}

		// Now we can add the option for removing pin
		Section.AddMenuEntryWithCommandList(FMDACommands::Get().RemoveBlendListPin, MDACommandList);
	}
	// Show a add pin option on the node menu
	else
	{
		Section.AddMenuEntryWithCommandList(FMDACommands::Get().AddBlendListPin, MDACommandList);
	}
}

void UAnimGraphNode_MDA::AddPinToBlendNode()
{
	FScopedTransaction Transaction(LOCTEXT("AddMDAPin", "AddMDAPin"));
	Modify();

	Node.AddPose();
	ReconstructNode();
	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(GetBlueprint());
}

void UAnimGraphNode_MDA::RemovePinFromBlendNode(UEdGraphPin* Pin)
{
	FScopedTransaction Transaction(LOCTEXT("RemoveMDAPin", "RemoveMDAPin"));
	Modify();

	FProperty* AssociatedProperty;
	int32 ArrayIndex;
	GetPinAssociatedProperty(GetFNodeType(), Pin, /*out*/ AssociatedProperty, /*out*/ ArrayIndex);

	if (ArrayIndex != INDEX_NONE)
	{
		//@TODO: ANIMREFACTOR: Need to handle moving pins below up correctly
		// setting up removed pins info
		RemovedPinArrayIndex = ArrayIndex;
		Node.RemovePose(ArrayIndex);
		// removes the selected pin and related properties in reconstructNode()
		// @TODO: Considering passing "RemovedPinArrayIndex" to ReconstructNode as the argument
		ReconstructNode();
		FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(GetBlueprint());
	}
}

void UAnimGraphNode_MDA::PostPlacedNewNode()
{
	Super::PostPlacedNewNode();

	// Make sure we start out with one inputs
	Node.AddPose();
	ReconstructNode();
}

TSharedPtr<SGraphNode> UAnimGraphNode_MDA::CreateVisualWidget()
{
	return SNew(SGraphNodeMDA, this);
}

void UAnimGraphNode_MDA::ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>& OldPins)
{
	Super::ReallocatePinsDuringReconstruction(OldPins);

	// Delete Pins by removed pin info 
	if (RemovedPinArrayIndex != INDEX_NONE)
	{
		RemovePinsFromOldPins(OldPins, RemovedPinArrayIndex);
		// Clears removed pin info to avoid to remove multiple times
		// @TODO : Considering receiving RemovedPinArrayIndex as an argument of ReconstructNode()
		RemovedPinArrayIndex = INDEX_NONE;
	}
}

void UAnimGraphNode_MDA::RemovePinsFromOldPins(TArray<UEdGraphPin*>& OldPins, int32 RemovedArrayIndex)
{
	TArray<FString> RemovedPropertyNames;
	TArray<FName> NewPinNames;

	// Store new pin names to compare with old pin names
	for (const auto& Pin : Pins)
	{
		NewPinNames.Add(Pin->PinName);
	}

	// don't know which pins are removed yet so find removed pins comparing NewPins and OldPins
	for (const auto& Pin : OldPins)
	{
		const FName OldPinName = Pin->PinName;
		if (!NewPinNames.Contains(OldPinName))
		{
			const FString OldPinNameStr = OldPinName.ToString();
			const int32 UnderscoreIndex = OldPinNameStr.Find(TEXT("_"), ESearchCase::CaseSensitive);
			if (UnderscoreIndex != INDEX_NONE)
			{
				FString PropertyName = OldPinNameStr.Left(UnderscoreIndex);
				RemovedPropertyNames.Add(MoveTemp(PropertyName));
			}
		}
	}

	for (int32 PinIdx = 0; PinIdx < OldPins.Num(); PinIdx++)
	{
		// Separate the pin name into property name and index
		const FString OldPinNameStr = OldPins[PinIdx]->PinName.ToString();

		const int32 UnderscoreIndex = OldPinNameStr.Find(TEXT("_"), ESearchCase::CaseSensitive);
		if (UnderscoreIndex != INDEX_NONE)
		{
			const FString PropertyName = OldPinNameStr.Left(UnderscoreIndex);

			if (RemovedPropertyNames.Contains(PropertyName))
			{
				const int32 ArrayIndex = FCString::Atoi(*(OldPinNameStr.Mid(UnderscoreIndex + 1)));
				// if array index is matched, removes pins 
				// and if array index is greater than removed index, decrease index
				if (ArrayIndex == RemovedArrayIndex)
				{
					OldPins[PinIdx]->MarkAsGarbage();
					OldPins.RemoveAt(PinIdx);
					--PinIdx;
				}
				else if (ArrayIndex > RemovedArrayIndex)
				{
					OldPins[PinIdx]->PinName = *FString::Printf(TEXT("%s_%d"), *PropertyName, ArrayIndex - 1);
				}
			}
		}
	}
}
#undef LOCTEXT_NAMESPACE
