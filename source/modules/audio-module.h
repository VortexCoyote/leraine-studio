#pragma once

#include "base/module.h"

#include <bass.h>
#include <bass_fx.h>

class AudioModule : public Module
{
public:
	
	virtual bool Tick(const float& InDeltaTime) override;

public:

	void LoadAudio(const std::string& InPath);
	
	void TogglePause();
	void SetPause(bool InPause);
	void ResetSpeed();
	void SetTimeMilliSeconds(const Time InTime);

	void MoveDelta(const int InDeltaMilliSeconds);
	void ChangeSpeed(const float InDeltaSpeed);

	double GetTimeSeconds();
	Time GetTimeMilliSeconds();
	Time GetSongLengthMilliSeconds();
	float GetPlaybackSpeed();
	
	[[nodiscard]] WaveFormData* GenerateAndGetWaveformData(const std::string& InPath);

	bool UsePitch = true;

private:

	const WaveFormData& SampleWaveFormData(const Time InTimePoint);

	WaveFormData* _ReadableWaveFormData = nullptr;

	double _CurrentTime = 0;
	float _Speed = 1.f;
	bool _Paused = true;

	float* _WaveFormData = nullptr;
	DWORD _SongByteLength;

	//relevant BASS variables
	int _Device = -1; // Default Sounddevice
	int _Freq = 44100; // Sample rate (Hz)

	HSAMPLE _StreamHandle; // Handle for open stream
};