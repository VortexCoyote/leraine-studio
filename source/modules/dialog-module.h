#include "base/module.h"

#include <functional>

/*
* /!\WARNING/!\
*
* this module contains an ugly workaround to abstract file dialogs
* this is primarely used for opening up a dialog ""outside"" imgui windows
*/

class DialogModule : public Module
{
public:

	bool Tick(const float& InDeltaTime) override;
	bool RenderBack(sf::RenderTarget* const InOutRenderTarget) override;

public:

	void OpenFolderDialog(std::function<void(const std::string&)> InSelectedFolderAction, const bool InShouldCallActionWhenClosing = false);
	void OpenFileDialog(const std::string& InFileExtension, std::function<void(const std::string&)> InSelectedFileAction, const bool InShouldCallActionWhenClosing = false);

	bool IsDialogOpen();

private:

	bool _IsOpen = false;
	bool _ShouldOpenDialog = false;
	bool _ShouldCallActionWhenClosing = false;

	std::string _FileExtension = "";

	sf::RenderTarget* _RenderWindow = nullptr;

	std::function<void(const std::string&)> _SelectedItemAction;

	enum class EDialogType
	{
		Folder,
		File,

		COUNT
	} _DialogType;
};