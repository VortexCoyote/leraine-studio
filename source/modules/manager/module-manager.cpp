#include "module-manager.h"

ModuleManager* ModuleManager::static_ModuleManager = nullptr;

#define ITERATE_MODULES(name) auto& name : static_ModuleManager->_Modules

void ModuleManager::StartUp()
{
	for (ITERATE_MODULES(moduleInstance))
		moduleInstance->StartUp();
}

void ModuleManager::Tick(const float& InDeltaTime)
{
	for (ITERATE_MODULES(moduleInstance))
		moduleInstance->Tick(InDeltaTime);
}

void ModuleManager::RenderBack(sf::RenderTarget* const InOutRenderTarget) 
{
	for (ITERATE_MODULES(moduleInstance))
		moduleInstance->RenderBack(InOutRenderTarget);
}

void ModuleManager::RenderFront(sf::RenderTarget* const InOutRenderTarget) 
{
	for (ITERATE_MODULES(moduleInstance))
		moduleInstance->RenderFront(InOutRenderTarget);
}

void ModuleManager::ProcessEvent(const sf::Event& InEvent)
{
	for (ITERATE_MODULES(moduleInstance))
		moduleInstance->ProcessEvent(InEvent);
}

void ModuleManager::ShutDown()
{
	for (ITERATE_MODULES(moduleInstance))
		moduleInstance->ShutDown();
}

void ModuleManager::Init()
{
	if (!static_ModuleManager)
		static_ModuleManager = new ModuleManager();
}

void ModuleManager::Destroy()
{
	for (ITERATE_MODULES(moduleInstance))
		delete moduleInstance;

	if (static_ModuleManager)
		delete static_ModuleManager;
}
