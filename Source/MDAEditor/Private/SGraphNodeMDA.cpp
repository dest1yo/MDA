// Copyright 2023 dest1yo. All Rights Reserved.

#include "SGraphNodeMDA.h"
#include "AnimGraphNode_MDA.h"
#include "GenericPlatform/ICursor.h"
#include "GraphEditorSettings.h"
#include "Internationalization/Internationalization.h"
#include "Layout/Margin.h"
#include "Misc/Optional.h"
#include "SlotBase.h"
#include "Types/SlateEnums.h"
#include "Widgets/SBoxPanel.h"

class SWidget;

/////////////////////////////////////////////////////
// SGraphNodeMDA

void SGraphNodeMDA::Construct(const FArguments& InArgs, UAnimGraphNode_MDA* InNode)
{
	this->GraphNode = Node = InNode;

	this->SetCursor(EMouseCursor::CardinalCross);

	this->UpdateGraphNode();

	SAnimationGraphNode::Construct(SAnimationGraphNode::FArguments(), InNode);
}

void SGraphNodeMDA::CreateInputSideAddButton(TSharedPtr<SVerticalBox> InputBox)
{
	TSharedRef<SWidget> AddPinButton = AddPinButtonContent(
		NSLOCTEXT("MDANode", "MDANodeAddPinButton", "Add pin"),
		NSLOCTEXT("MDANode", "MDANodeAddPinButton_Tooltip", "Adds a input pose to the node"),
		false);

	FMargin AddPinPadding = Settings->GetInputPinPadding();
	AddPinPadding.Top += 6.0f;

	InputBox->AddSlot()
		.AutoHeight()
		.VAlign(VAlign_Center)
		.Padding(AddPinPadding)
		[
			AddPinButton
		];
}

FReply SGraphNodeMDA::OnAddPin()
{
	Node->AddPinToBlendNode();

	return FReply::Handled();
}
