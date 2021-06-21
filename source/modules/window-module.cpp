#include "window-module.h"

#include "imgui.h"
#include "imgui-SFML.h"

#include <Windows.h>

bool WindowModule::StartUp() 
{
    return false;
}

bool WindowModule::ProcessEvent(const sf::Event& InEvent) 
{
    if(InEvent.type == sf::Event::LostFocus)
    {
        std::cout << "lost focus" << std::endl;
    }

    if(InEvent.type == sf::Event::GainedFocus)
    {
        std::cout << "gained focus" << std::endl;
    }

    return false;
}

void WindowModule::Init(sf::RenderWindow* const InOutRenderWindow, bool* OutShouldClose, std::function<void()> InWindowWork) 
{
    _RenderWindow = InOutRenderWindow;
    
    _ShouldExitProgram = OutShouldClose;
    _WindowWork = InWindowWork;
}

bool WindowModule::Tick(const float& InDeltaTime) 
{
    //edge cases edge cases edge cases edge cases edge cases edge cases edge cases edge cases edge cases edge cases edge cases edge cases edge cases edge cases edge cases edge cases edge cases edge cases 

    ImGuiWindowFlags flags = ImGuiWindowFlags_None;

	flags |= ImGuiWindowFlags_NoMove;
	flags |= ImGuiWindowFlags_NoScrollbar;
	flags |= ImGuiWindowFlags_NoScrollWithMouse;
	flags |= ImGuiWindowFlags_NoCollapse;
	flags |= ImGuiWindowFlags_NoSavedSettings;
    flags |= ImGuiWindowFlags_MenuBar;
    flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;

	ImGui::SetNextWindowPos({ 0, 0 });
    ImGui::SetNextWindowSize({float(_RenderWindow->getSize().x),float(_RenderWindow->getSize().y)});
    ImGui::SetNextWindowBgAlpha(0.0);

    bool isOpen = true;
    ImGui::Begin("Leraine Studio", &isOpen, flags);

    ImGui::GetIO().WantCaptureMouse = false;

    if(ImGui::IsItemHovered())
    {
        ImGui::GetIO().WantCaptureMouse = true;

        if(ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            _MovingWindow = true;
    }
    
    _CurrentWindowWidth = (int)(ImGui::GetWindowSize().x);
    _CurrentWindowHeight = (int)(ImGui::GetWindowSize().y);
    
    _WindowWork();

    if(!isOpen)
    {
        //do close work (auto save?)

        _RenderWindow->close();
        *_ShouldExitProgram = true;
    }

    ImGui::End();
    
    if(_CurrentWindowWidth != _PreviousWindowWidth || _CurrentWindowHeight != _PreviousWindowHeight)
    {
        _RenderWindow->setSize(sf::Vector2u(_CurrentWindowWidth, _CurrentWindowHeight));

        sf::FloatRect visibleArea(0, 0, _CurrentWindowWidth, _CurrentWindowHeight);
        _RenderWindow->setView(sf::View(visibleArea));
        
        if(ImGui::GetMousePos().x <= 16 + (int)ImGui::GetMouseDragDelta().x)
            _RenderWindow->setPosition(sf::Vector2i(_RenderWindow->getPosition().x + (int)ImGui::GetMouseDragDelta().x, _RenderWindow->getPosition().y));
        
        if(ImGui::GetMousePos().y <= 16 + (int)ImGui::GetMouseDragDelta().y)
            _RenderWindow->setPosition(sf::Vector2i(_RenderWindow->getPosition().x, _RenderWindow->getPosition().y + (int)ImGui::GetMouseDragDelta().y));
    }

    _PreviousWindowWidth = _CurrentWindowWidth;
    _PreviousWindowHeight = _CurrentWindowHeight;

    if(_MovingWindow)
    {
        _RenderWindow->setPosition(sf::Vector2i(_RenderWindow->getPosition().x + (int)ImGui::GetMouseDragDelta().x, 
                                                _RenderWindow->getPosition().y + (int)ImGui::GetMouseDragDelta().y));

        if(ImGui::IsMouseReleased(ImGuiMouseButton_Left))
            _MovingWindow = false;

    }

    return true;
}