#include "dialog-module.h"

#include "imgui.h"
#include "../utilities/imgui/addons/imguifilesystem/imguifilesystem.h"

bool DialogModule::Tick(const float& InDeltaTime)
{
	if(!_RenderWindow)
		return false;

	ImGuiWindowFlags flags = ImGuiWindowFlags_None;

	flags |= ImGuiWindowFlags_NoResize;
	flags |= ImGuiWindowFlags_NoMove;
	flags |= ImGuiWindowFlags_NoScrollbar;
	flags |= ImGuiWindowFlags_NoScrollWithMouse;
	flags |= ImGuiWindowFlags_NoCollapse;
	flags |= ImGuiWindowFlags_AlwaysAutoResize;
	flags |= ImGuiWindowFlags_NoBackground;
	flags |= ImGuiWindowFlags_NoSavedSettings;
	flags |= ImGuiWindowFlags_NoMouseInputs;
	flags |= ImGuiWindowFlags_HorizontalScrollbar;
	flags |= ImGuiWindowFlags_NoFocusOnAppearing;
	flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
	flags |= ImGuiWindowFlags_AlwaysVerticalScrollbar;
	flags |= ImGuiWindowFlags_AlwaysHorizontalScrollbar;
	flags |= ImGuiWindowFlags_AlwaysUseWindowPadding;
	flags |= ImGuiWindowFlags_NoNavInputs;
	flags |= ImGuiWindowFlags_NoNavFocus;
	flags |= ImGuiWindowFlags_UnsavedDocument;
	flags |= ImGuiWindowFlags_NoNav;
	flags |= ImGuiWindowFlags_NoDecoration;
	flags |= ImGuiWindowFlags_NoInputs;

	ImGui::SetNextWindowPos({ 999999, 999999 });

	bool isWindowOpened;
	ImGui::Begin("DialogModuleWindow", &isWindowOpened, flags);

	static ImGuiFs::Dialog dialog;
	
	const float size = 1.25f;

	float width = _RenderWindow->getView().getSize().x;
	float height = _RenderWindow->getView().getSize().y;

	std::string chosenPath; 
	switch (_DialogType)
	{
	case DialogModule::EDialogType::Folder:
		chosenPath = dialog.chooseFolderDialog(_ShouldOpenDialog, 0, 0, { width / size, height / size }, { width / (10.f), height / (10.f) }, 1.f);
		break;
	
	case DialogModule::EDialogType::File:
		chosenPath = dialog.chooseFileDialog(_ShouldOpenDialog, 0, _FileExtension.c_str(), 0, { width  / size, height/ size }, { width / (10.f), height / (10.f) }, 1.f);
		break;

	default:
		break;
	}
	
	if (chosenPath.size() != 0)
	{
		_SelectedItemAction(chosenPath);
		_IsOpen = false;
	}

	if (dialog.hasUserJustCancelledDialog())
		_IsOpen = false;
	
	ImGui::End();

	if (_ShouldOpenDialog)
		_ShouldOpenDialog = false;

	return true;
}

bool DialogModule::RenderBack(sf::RenderTarget* const InOutRenderTarget) 
{
	_RenderWindow = InOutRenderTarget;

	return true;
}

void DialogModule::OpenFolderDialog(std::function<void(const std::string&)> InSelectedFolderAction)
{
	_ShouldOpenDialog = true;
	_IsOpen = true;
	_SelectedItemAction = InSelectedFolderAction;

	_DialogType = EDialogType::Folder;
}

void DialogModule::OpenFileDialog(const std::string& InFileExtension, std::function<void(const std::string&)> InSelectedFileAction)
{
	_ShouldOpenDialog = true;
	_IsOpen = true;
	_SelectedItemAction = InSelectedFileAction;

	_FileExtension = InFileExtension;

	_DialogType = EDialogType::File;
}

bool DialogModule::IsDialogOpen()
{
	return _IsOpen;
}
