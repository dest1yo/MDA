// Copyright 2023 dest1yo. All Rights Reserved.

#pragma once
#include "AnimGraphNode_MDA.h"
#include "AnimationNodes/SAnimationGraphNode.h"
#include "Input/Reply.h"
#include "Templates/SharedPointer.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class SVerticalBox;
class UAnimGraphNode_MDA;

class SGraphNodeMDA : public SAnimationGraphNode
{
public:
	SLATE_BEGIN_ARGS(SGraphNodeMDA){}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UAnimGraphNode_MDA* InNode);

	// The node that we represent
	UAnimGraphNode_MDA* Node;

protected:
	// SGraphNode interface
	virtual void CreateInputSideAddButton(TSharedPtr<SVerticalBox> InputBox) override;
	virtual FReply OnAddPin() override;
	// End of SGraphNode interface
};
