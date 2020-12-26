#pragma once

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
	void ScrollShortcutRoutines();
	void InputActions();
	void ApplyDeltaToZoom(const float InDelta);
	void UpdateCursor();

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

private: //program data ownership

	TimefieldRenderGraph _ChartRenderGraph;
	TimefieldRenderGraph _PreviewRenderGraph;

	Chart* _SelectedChart = nullptr;

	float _ZoomLevel = 1.0f;
	int _CurrentSnap = 2;

private: //meta data ownership

	sf::Clock _DeltaClock;
	sf::RenderWindow* _RenderWindow;
	WindowMetrics _WindowMetrics;
};