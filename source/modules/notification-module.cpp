#include "notification-module.h"

#include <algorithm>

#include "imgui.h"

bool NotificationModule::Tick(const float& InDeltaTime) 
{
    if(!_RenderWindow)
        return false;

    NotificationMessage::Message* messageToRemove = nullptr;

    int placement = 0;
    for(auto& message : NotificationMessage::Messages)
    {
        ImGuiWindowFlags windowFlags = 0;
	    windowFlags |= ImGuiWindowFlags_NoTitleBar;
	    windowFlags |= ImGuiWindowFlags_NoMove;
	    windowFlags |= ImGuiWindowFlags_NoResize;
	    windowFlags |= ImGuiWindowFlags_NoCollapse;
	    windowFlags |= ImGuiWindowFlags_AlwaysAutoResize;
	    windowFlags |= ImGuiWindowFlags_NoScrollbar;

	    bool open = true;

	    ImGui::Begin(std::to_string((int)&message).c_str(), &open, windowFlags);    

        ImGui::Text(message.NotiMessage.c_str());

	    ImGui::SetWindowPos({ _RenderWindow->getView().getSize().x - (ImGui::GetWindowWidth() + 8.f), _StartY + 48.f * float(placement)});
        
        ImGui::End();

        message.LifeTime -= InDeltaTime;

        if(message.LifeTime <= 0.f)
        {
            messageToRemove = &message;
            placement--;
        }
        
        placement++;
    }

    if(messageToRemove)
        NotificationMessage::Messages.erase(std::remove(NotificationMessage::Messages.begin(), NotificationMessage::Messages.end(), *messageToRemove), NotificationMessage::Messages.end());

    return true;
}

bool NotificationModule::RenderBack(sf::RenderTarget* const InOutRenderTarget) 
{
    if(!_RenderWindow)
        _RenderWindow = (sf::RenderWindow*)InOutRenderTarget;

    return true;
}

void NotificationModule::SetStartY(const int InY) 
{
    _StartY = InY;    
}
