#pragma once

#include <functional>

#include "base/module.h"

class TimefieldRenderModule : public Module
{
public: //module overrides

	bool StartUp() override;
	bool Tick(const float& InDeltaTime) override;

	bool RenderBack(sf::RenderTarget* const InOutRenderTarget) override;
	bool RenderFront(sf::RenderTarget* const InOutRenderTarget) override;

public: //rendering

	void RenderTimefieldGraph(sf::RenderTarget* const InOutRenderTarget, TimefieldRenderGraph& InOutTimefieldRenderGraph, const Time InTime, const float InZoomLevel, const bool InRegisterToOnscreenNotes = true);	
	sf::RenderTexture* const GetRenderedTimefieldGraphSegment(TimefieldRenderGraph& InOutTimefieldRenderGraph, const Time InTime, const float InZoomLevel);

	void RenderBeatLine(sf::RenderTarget* const InOutRenderTarget, const Time InBeatTimePoint, const int InBeatSnap, const Time InTime, const float InZoomLevel);
	void RenderReceptors(sf::RenderTarget* const InOutRenderTarget, const int InBeatSnap);

public: //data gathering

	int GetScreenPointFromTime(const Time InTimePoint, const Time InTime, const float InZoomLevel);
	Time GetTimeFromScreenPoint(const int InScreenPointY, const Time InTime, const float InZoomLevel, const bool InNoteTimePivot = false);
	Column GetColumnFromScreenPoint(const int InScreenPointX);

	Time GetWindowTimePointBegin(const Time InTime, const float InZoomLevel);
	Time GetWindowTimePointEnd(const Time InTime, const float InZoomLevel);

	void GetOverlappedOnScreenNotes(const Column InColumn, const int InScreenPointY, std::vector<const Note*>& OutNoteCollection);

	Skin& GetSkin();
	const TimefieldMetrics& GetTimefieldMetrics();

public: //data setting

	void InitializeResources(const int InKeyAmount, const std::filesystem::path& InSkinFolderPath);
	void UpdateMetrics(const WindowMetrics& InWindowMetrics);

private: //data ownership

	sf::RenderTexture _NoteRenderLayer;
	sf::RenderTexture _HoldRenderLayer;

	sf::Sprite _NoteRenderLayerSprite;
	sf::Sprite _HoldRenderLayerSprite;

	sf::RenderTexture _SegmentedRenderTexture;
	sf::RenderTexture _ResultingSegmentedRenderTexture;
	sf::Sprite _SegmentedSprite;


	TimefieldMetrics _TimefieldMetrics;
	WindowMetrics _WindowMetrics;

	int _KeyAmount;

	struct _OnScreenNote 
	{ 
		const Note* m_Note; 
		const Column m_Column; 
		const int OnScreenY; 
	};

	std::vector<_OnScreenNote> _OnScreenNotes;

	Skin _Skin;
};