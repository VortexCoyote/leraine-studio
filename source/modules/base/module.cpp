#include "module.h"

Module::Module()
{

}

Module::~Module()
{

}

bool Module::StartUp()
{
	return false;
}

bool Module::Tick(const float& InDeltaTime)
{
	return false;
}

bool Module::RenderBack(sf::RenderTarget* const InOutRenderTarget) 
{
	return false;
}

bool Module::RenderFront(sf::RenderTarget* const InOutRenderTarget) 
{
	return false;
}

bool Module::ProcessEvent(const sf::Event& InEvent)
{
	return false;
}

bool Module::ShutDown()
{
	return false;
}
