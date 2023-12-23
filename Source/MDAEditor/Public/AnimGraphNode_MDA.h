// Copyright 2023 dest1yo. All Rights Reserved.

#pragma once
#include "SGraphNodeMDA.h"
#include "AnimGraphNode_Base.h"
#include "AnimNode_MDA.h"
#include "AnimGraphNode_MDA.generated.h"

class SGraphNodeMDA;

UCLASS(meta = (Keywords = "MDA"))
class MDAEDITOR_API UAnimGraphNode_MDA : public UAnimGraphNode_Base
{
	GENERATED_UCLASS_BODY()
	
	UPROPERTY(EditAnywhere, Category = Settings)
	FAnimNode_MDA Node;

	// Adds a new pose pin
	//@TODO: Generalize this behavior (returning a list of actions/delegates maybe?)
	virtual void AddPinToBlendNode();
	virtual void RemovePinFromBlendNode(UEdGraphPin* Pin);
	virtual void ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>& OldPins) override;

	//~ Begin UEdGraphNode Interface.
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual FText GetTooltipText() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual void PostPlacedNewNode() override;
	virtual TSharedPtr<SGraphNode> CreateVisualWidget() override;	// Bind our widget
	// virtual FLinearColor GetNodeBodyTintColor() const override;	// TODO: research
	//~ End UEdGraphNode Interface.

	//~ Begin UAnimGraphNode_Base Interface
	virtual FString GetNodeCategory() const override;
	//~ End UAnimGraphNode_Base Interface

	// UK2Node interface
	virtual void GetNodeContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const override;
	// End of UK2Node interface

private:
	int32 RemovedPinArrayIndex;

	// removes removed pins and adjusts array indices of remained pins
	void RemovePinsFromOldPins(TArray<UEdGraphPin*>& OldPins, int32 RemovedArrayIndex);
};
