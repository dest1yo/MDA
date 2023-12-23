// Copyright 2023 dest1yo. All Rights Reserved.

#include "MDAEditor.h"
#include "MDACommands.h"
#include "AnimGraphNode_MDA.h"
#include "BlueprintEditor.h"

#define LOCTEXT_NAMESPACE "FMDAEditorModule"

void FMDAEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FMDACommands::Register();

	// 创建UICommandList
	MDACommandList = MakeShareable(new FUICommandList);

	// 为菜单命令映射具体操作
	OnCreateMDACommands();
}

void FMDAEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	FMDACommands::Unregister();
}

void FMDAEditorModule::TestCommand()
{
	FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("Command Executed."));
}

void FMDAEditorModule::OnCreateMDACommands()
{
	// Add pose pin
	MDACommandList->MapAction( FMDACommands::Get().AddBlendListPin,
	FExecuteAction::CreateRaw( this, &FMDAEditorModule::OnAddPosePin ),
	FCanExecuteAction::CreateRaw( this, &FMDAEditorModule::CanAddPosePin )
	);

	// Remove pose pin
	MDACommandList->MapAction( FMDACommands::Get().RemoveBlendListPin,
	FExecuteAction::CreateRaw( this, &FMDAEditorModule::OnRemovePosePin ),
	FCanExecuteAction::CreateRaw( this, &FMDAEditorModule::CanRemovePosePin )
	);
}

FBlueprintEditor* FMDAEditorModule::GetFocusedBlueprintEditor()
{
	// Iterate over all assets being edited
	TArray<UObject*> Assets = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->GetAllEditedAssets();

	for (const auto& Asset : Assets)
	{
		// Get the editor of asset
		IAssetEditorInstance* AssetEditor = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->FindEditorForAsset(Asset, false);

		if (FAssetEditorToolkit* Editor = static_cast<FAssetEditorToolkit*>(AssetEditor); Editor->GetTabManager()->GetOwnerTab()->IsForeground())
		{
			return static_cast<FBlueprintEditor*>(Editor);
		}
	}

	return nullptr;
}

TSharedPtr<SGraphEditor> FMDAEditorModule::GetFocusedGraphEditor()
{
	const FBlueprintEditor* BlueprintEditor = GetFocusedBlueprintEditor();
	const TSharedPtr<SGraphEditor> FocusedGraphEd = SGraphEditor::FindGraphEditorForGraph(BlueprintEditor->GetFocusedGraph());

	// Check it
	if (FocusedGraphEd.IsValid())
		return FocusedGraphEd;

	return {};
}

void FMDAEditorModule::OnAddPosePin()
{
	// Get current focused graph editor
	const TSharedPtr<SGraphEditor> FocusedGraphEd = GetFocusedGraphEditor();

	// Get the selected node
	UAnimGraphNode_MDA* MDANode = Cast<UAnimGraphNode_MDA>(FocusedGraphEd->GetGraphNodeForMenu());

	// Add pin
	MDANode->AddPinToBlendNode();
}

void FMDAEditorModule::OnRemovePosePin()
{
	// Get current focused graph editor
	const TSharedPtr<SGraphEditor> FocusedGraphEd = GetFocusedGraphEditor();

	// Get the selected pin
	UEdGraphPin* SelectedPin = FocusedGraphEd->GetGraphPinForMenu();

	// Get the selected node
	UAnimGraphNode_MDA* MDANode = Cast<UAnimGraphNode_MDA>(SelectedPin->GetOwningNode());

	// Remove pin
	MDANode->RemovePinFromBlendNode(SelectedPin);

	// Update the graph so that the node will be refreshed
	FocusedGraphEd->NotifyGraphChanged();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FMDAEditorModule, MDAEditor)
