#include "popup-module.h"

#include "imgui.h"


bool PopupModule::StartUp() 
{
    return true;    
}

bool PopupModule::Tick(const float& InDeltaTime) 
{
    if(!_RenderWindow)
        return false;

    if(!_Open)
        return false;
    
    ImGui::OpenPopup(_PopupName.c_str());
    ImGui::GetIO().WantCaptureMouse = true;

    ImGuiWindowFlags windowFlags = 0;
	windowFlags |= ImGuiWindowFlags_NoTitleBar;
	windowFlags |= ImGuiWindowFlags_NoMove;
	windowFlags |= ImGuiWindowFlags_NoResize;
	//windowFlags |= ImGuiWindowFlags_NoCollapse;
	//windowFlags |= ImGuiWindowFlags_NoBackground;
	windowFlags |= ImGuiWindowFlags_NoScrollbar;
	windowFlags |= ImGuiWindowFlags_AlwaysAutoResize;

	bool open = true;
	if (ImGui::BeginPopupModal(_PopupName.c_str(), &open, windowFlags))
	{
        _Work(_Open);

		ImGui::SetWindowPos({ _RenderWindow->getView().getSize().x / 2.f - ImGui::GetWindowSize().x / 2.f, _RenderWindow->getView().getSize().y / 2.f - ImGui::GetWindowSize().y / 2.f });
		ImGui::EndPopup();
	}

    return true;
}

bool PopupModule::RenderBack(sf::RenderTarget* const InOutRenderTarget) 
{
    if(!_RenderWindow)
        _RenderWindow = (sf::RenderWindow*)InOutRenderTarget;

    return false;
}

bool PopupModule::ProcessEvent(const sf::Event& InEvent) 
{
    if(InEvent.type == sf::Event::KeyPressed)
    {
        if(InEvent.key.code == sf::Keyboard::Key::Escape)
            _Open = false;
    }
}

void PopupModule::OpenPopup(const std::string& InPopupName, std::function<void(bool&)> InWork) 
{
    _PopupName = InPopupName;
    _Work = InWork;

    _Open = true;
}
