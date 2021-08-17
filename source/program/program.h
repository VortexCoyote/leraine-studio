#pragma once

#include "../structures/configuration.h"
#include "../modules/manager/module-manager.h"

class Program
{
public: //inner program sequences

	void InnerStartUp();
	void InnerTick();
	void InnerRender(sf::RenderTarget* const InOutRenderTarget);
	void InnerShutDown();

public: //abstractions

	void MenuBar();
	void SetUpMetadata();
	void ShowShortCuts();
	void GoToTimePoint();
	void ScrollShortcutRoutines();
	void InputActions();
	void GlobalInputActions();
	void ApplyDeltaToZoom(const float InDelta);
	void UpdateCursor();
	void OpenChart(const std::string InPath);
	void SetConfig(Configuration config);

public: //meta program sequences

	Program();
	~Program();

	void Init();
	bool HandleEvents();
	void RegisterModules();
	void UpdateWindowMetrics();

	void StartUp();
	void Tick();
	void Render();
	void ShutDown();

private: //meta data ownership
	
	sf::Clock _DeltaClock;
	sf::RenderWindow* _RenderWindow;
	WindowMetrics _WindowMetrics;
	
	bool _ShouldExitProgram = false;
};