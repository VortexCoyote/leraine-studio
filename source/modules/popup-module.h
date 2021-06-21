#pragma once

#include <string>
#include <functional>

#include "base/module.h"

class PopupModule : public Module
{
public:

	bool StartUp() override;
	bool Tick(const float& InDeltaTime) override;
    bool RenderBack(sf::RenderTarget* const InOutRenderTarget) override;

public:

    void OpenPopup(const std::string& InPopupName, std::function<void(bool&)> InWork);

private:

    bool _Open = false;
    std::string _PopupName;

    std::function<void(bool&)> _Work;

    sf::RenderWindow* _RenderWindow = nullptr;
};