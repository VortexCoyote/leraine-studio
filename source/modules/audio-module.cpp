#include "audio-module.h"

bool AudioModule::Tick(const float& InDeltaTime)
{
	BASS_Update(_StreamHandle);
	_CurrentTime = GetTimeSeconds();

	return true;
}

void AudioModule::LoadAudio(const std::string& InPath)
{
	BASS_Free();
	BASS_Init(_Device, _Freq, 0, 0, NULL);

	_StreamHandle = BASS_FX_TempoCreate(BASS_StreamCreateFile(FALSE, InPath.c_str(), 0, 0, BASS_STREAM_DECODE | BASS_STREAM_PRESCAN), BASS_FX_FREESOURCE);

	auto error = BASS_ErrorGetCode();
	if (error != 0)
	{
		std::cout << BASS_ErrorGetCode() << std::endl;
	}
	
	BASS_ChannelPlay(_StreamHandle, FALSE);
	BASS_ChannelPause(_StreamHandle);

	BASS_ChannelSetAttribute(_StreamHandle, BASS_ATTRIB_TEMPO_OPTION_SEQUENCE_MS, 32);
	BASS_ChannelSetAttribute(_StreamHandle, BASS_ATTRIB_TEMPO_OPTION_SEEKWINDOW_MS, 4);

	ResetSpeed();

	_Paused = true;
}

void AudioModule::TogglePause()
{
	_Paused = !_Paused;

	SetPause(_Paused);
}

void AudioModule::SetPause(bool InPause)
{
	_Paused = InPause;

	if (_Paused)
		BASS_ChannelPause(_StreamHandle);
	else
		BASS_ChannelPlay(_StreamHandle, FALSE);

	_CurrentTime = GetTimeSeconds();
}

void AudioModule::ResetSpeed()
{
	_Speed = 1.f;

	BASS_CHANNELINFO info;
	BASS_ChannelGetInfo(_StreamHandle, &info);


	//if (usePitch == true)
	//{
		BASS_ChannelSetAttribute(_StreamHandle, BASS_ATTRIB_FREQ, float(info.freq) * _Speed);
		BASS_ChannelSetAttribute(_StreamHandle, BASS_ATTRIB_TEMPO, 0);
	//}
	//else
	//{
	//	BASS_ChannelSetAttribute(myStreamHandle, BASS_ATTRIB_TEMPO, (mySpeed - 1.f) * 100.f);
	//}
}

void AudioModule::SetTimeMilliSeconds(const Time InTime)
{
	_CurrentTime = double(InTime) / 1000.0;

	BASS_ChannelSetPosition(_StreamHandle, BASS_ChannelSeconds2Bytes(_StreamHandle, _CurrentTime), BASS_POS_BYTE);
}

void AudioModule::MoveDelta(const int InDeltaMilliSeconds)
{
	_CurrentTime += double(InDeltaMilliSeconds) / 1000.0;

	BASS_ChannelSetPosition(_StreamHandle, BASS_ChannelSeconds2Bytes(_StreamHandle, _CurrentTime), BASS_POS_BYTE);
}

void AudioModule::ChangeSpeed(const float InDeltaSpeed)
{
	_Speed += InDeltaSpeed;
	
	if (_Speed < 0.05f)
		_Speed = 0.05f;

	if (_Speed > 2.f)
		_Speed = 2.f;

	BASS_CHANNELINFO info;
	BASS_ChannelGetInfo(_StreamHandle, &info);

	//if (usePitch == true)
	//{
		BASS_ChannelSetAttribute(_StreamHandle, BASS_ATTRIB_FREQ, float(info.freq) * _Speed);
		BASS_ChannelSetAttribute(_StreamHandle, BASS_ATTRIB_TEMPO, 0);
	//}
	//else
	//{
	//	BASS_ChannelSetAttribute(myStreamHandle, BASS_ATTRIB_TEMPO, (mySpeed - 1.f) * 100.f);
	//}
}

double AudioModule::GetTimeSeconds()
{
	return BASS_ChannelBytes2Seconds(_StreamHandle, BASS_ChannelGetPosition(_StreamHandle, BASS_POS_BYTE));
}

Time AudioModule::GetTimeMilliSeconds()
{
	return _CurrentTime * 1000;
}

Time AudioModule::GetSongLengthMilliSeconds() 
{
	return BASS_ChannelBytes2Seconds(_StreamHandle, BASS_ChannelGetLength(_StreamHandle, BASS_POS_BYTE)) * 1000;
}
