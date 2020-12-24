#pragma once

#include <functional>

#include "base/module.h"

class TimefieldRenderModule : public Module
{
public:

	bool StartUp() override;
	bool Tick(const float& InDeltaTime) override;

public: //rendering

	void RenderTimefieldGraph(sf::RenderTarget* const InOutRenderTarget, TimefieldRenderGraph& InOutTimefieldRenderGraph, const Time InTime, const float InZoomLevel, const bool InRegisterToOnscreenNotes = true);
	void RenderBeatLine(sf::RenderTarget* const InOutRenderTarget, const Time InBeatTimePoint, const int InBeatSnap, const Time InTime, const float InZoomLevel);

	void RenderTimefieldBackground(sf::RenderTarget* const InOutRenderTarget);
	void RenderTimefieldForeground(sf::RenderTarget* const InOutRenderTarget);

public: //data gathering

	int GetScreenTimePoint(const Time InTimePoint, const Time InTime, const float InZoomLevel);
	Time GetTimeFromScreenPoint(const int InScreenPointY, const Time InTime, const float InZoomLevel, const bool InNoteTimePivot = false);
	Column GetColumnFromScreenPoint(const int InScreenPointX);

	Time GetWindowTimePointBegin(const Time InTime, const float InZoomLevel);
	Time GetWindowTimePointEnd(const Time InTime, const float InZoomLevel);

	void GetOverlappedOnScreenNotes(const Column InColumn, const int InScreenPointY, std::vector<const Note*>& OutNoteCollection);

	Skin& GetSkin();

public: //data setting

	void SetKeyAmount(const int InKeyAmount);
	void UpdateMetrics(const WindowMetrics& InWindowMetrics);

private: //data ownership

	sf::RenderTexture _NoteRenderLayer;
	sf::RenderTexture _HoldRenderLayer;

	sf::Sprite _NoteRenderLayerSprite;
	sf::Sprite _HoldRenderLayerSprite;

	TimefieldMetrics _TimefieldMetrics;
	WindowMetrics _WindowMetrics;

	int _KeyAmount;

	struct _OnScreenNote 
	{ 
		const Note* Note; 
		const Column Column; 
		const int OnScreenY; 
	};

	std::vector<_OnScreenNote> _OnScreenNotes;

	Skin _Skin;
};