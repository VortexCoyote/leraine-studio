#pragma once

#include <map>
#include <vector>
#include <typeindex>

#include <SFML/System/Clock.hpp>

#include "../base/module.h"

#define MOD(name) ModuleManager::Get<name>()

class ModuleManager
{
public: //module routines

	static void StartUp();

	static void Tick(const float& InDeltaTime);
	static void RenderBack(sf::RenderTarget* const InOutRenderTarget);
	static void RenderFront(sf::RenderTarget* const InOutRenderTarget);
	static void ProcessEvent(const sf::Event& InEvent);

	static void ShutDown();

public: //accesors

	template<class T>
	static void Register()
	{
		static_ModuleManager->_ModuleHashLookUpTable[typeid(T).hash_code()] = static_ModuleManager->_Modules.size();
		static_ModuleManager->_Modules.push_back(new T());
	}
	
	template<class T>
	static T& Get()
	{
		return *static_cast<T*>(static_ModuleManager->_Modules[static_ModuleManager->_ModuleHashLookUpTable[typeid(T).hash_code()]]);
	}

public: //meta

	static void Init();
	static void Destroy();

private: //data ownership

	std::vector<Module*> _Modules;
	std::map<size_t, size_t> _ModuleHashLookUpTable;

	static ModuleManager* static_ModuleManager;
};