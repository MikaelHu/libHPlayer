#pragma once

#ifdef HPLAYER_EXPORTS
#define HPLAYER_API __declspec(dllexport)
#else
#define HPLAYER_API __declspec(dllimport)
#endif

#include <string>
#include <iostream>
#include <unordered_map>
#include <mutex>
#include "SDLAudio.h"

using std::string;


class HPLAYER_API SDLAudioPlayer {
public:
	enum eCurrentPlayState {
		e_UNDEF = -1,
		eStop,
		ePause,
		ePlaying,
		eStateNUM
	};

public:
	SDLAudioPlayer(SDLAudioOpt_t audio_opt = { 44100, 2, 1024, AUDIO_S16LSB });
	virtual ~SDLAudioPlayer();

	// get the number of available devices exposed by the current driver.
	//iscapture: 0表示播放，非0表示录制
	int GetNumAudioDevices(int iscapture);
	// get the human-readable name of a specific audio device.
	const char* GetAudioDeviceName(int index, int iscapture);

	//return:0 success, -1 failure.
	virtual int Start() { return 0; };
	virtual int Stop() { return 0; };
	virtual int Pause() { return 0; };
	virtual int Resume() { return 0; };
	// get the current audio state.
	virtual eCurrentPlayState Status() { return e_UNDEF; };

	// return 0 on success, or <0 on error.
	virtual int Write(void* buf, int len) { return 0; };
	// delay ms (millisecond)
	void Delay(Uint32 ms);
	string GetLastErr() { return lastErr_; }

protected:
	void Init();

protected:
	int inited_;
	string	lastErr_;
	SDLAudioOpt_t audioOption_;
	SDL_AudioSpec audioSpec_;
	eCurrentPlayState state_ = eStop;

};

class HPLAYER_API SDLQueueAudioPlayer : public SDLAudioPlayer {
public:
	SDLQueueAudioPlayer(SDLAudioOpt_t audio_opt = { 44100, 2, 1024, AUDIO_S16LSB });
	virtual ~SDLQueueAudioPlayer();

	// return:0 success, -1 failure.
	virtual int Start();
	virtual int Stop();
	virtual int Pause();
	virtual int Resume();
	virtual eCurrentPlayState Status();

	// return 0 on success, or -1 on error.
	virtual int Write(void* buf, int len);
	// drop any queued audio data
	void Clean();

protected:
	void Init();
	// get the number of bytes of still-queued audio.
	Uint32 GetQueuedAudioSize();
	// dequeue more audio on non-callback devices.
	Uint32 DequeueAudio(void *data, Uint32 len);

protected:
	SDL_AudioDeviceID deviceID_;
};

using std::unordered_map;

class Channel;
class HPLAYER_API SDLNonQueueAudioPlayer : public SDLAudioPlayer {
	typedef std::unordered_map<string, Channel*> UM_AUDIO_CH;
public:
	SDLNonQueueAudioPlayer(SDLAudioOpt_t audio_opt = { 44100, 2, 1024, AUDIO_S16LSB });
	virtual ~SDLNonQueueAudioPlayer();

	//return:0 success, -1 failure.
	virtual int Start();
	virtual int Stop();
	virtual int Pause();
	virtual int Resume();
	virtual eCurrentPlayState Status();

	// return >0 actually writen data, -1 device unready, -2 channel buffer is full.
	// -3 channel not found
	virtual int Write(void* buf, int len, string ch_name);

	void Clean(string ch_name);
	void CleanAll();
	int NewChannel(string ch_name);
	int RemoveChannel(string ch_name);
	int RemoveAllChannels();

	bool ChannelEmpty(string ch_name);
	bool ChannelFull(string ch_name);

protected:
	void Init();
	Channel* FindChannel(string ch_name);
	static void FeedAudio(void* udata, unsigned char* stream, int len);
	bool NeedMix(string ch_name, int len);
	bool IsSilent(unsigned char* src, int len);

protected:
	UM_AUDIO_CH channelsMap_;
	std::mutex mutex_;
};