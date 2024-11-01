#pragma once

#include <unordered_map>
#include <string>
#include <mutex>
#include <iostream>
#include <SDL.h>

#include "Channel.h"
#include "hlring/rbuf.h"



using std::string;

typedef struct SDLAudioOpt {
    int freq;
    int channels;
    int samples;
    int format;
}SDLAudioOpt_t, *pSDLAudioOpt_t;


class SDLNonQueueAudioPlayer {
	enum eCurrentPlayState { 
		e_UNDEF = -1,
		eStop, 
		ePause, 
		ePlaying,
		eStateNUM
	};

public:
    SDLNonQueueAudioPlayer(SDLAudioOpt_t sdl_audio_opt = { 44100, 2, 1024, AUDIO_S16LSB });
	~SDLNonQueueAudioPlayer();

	//return:0 success, -1 failure.
    int Start();    
    int Stop();
    int Pause();
    int Resume();

	// return >0 actually writen data, -1 device unready, -2 channel buffer is full.
	// -3 channel not found
    int Write(void* buf, int len, string ch_name);

    void Clean(string ch_name);
	void CleanAll();
    int NewChannel(string ch_name);
    int RemoveChannel(string ch_name);
	int RemoveAllChannels();    

	bool ChannelEmpty(string ch_name);
	bool ChannelFull(string ch_name);

	string GetLastErr() { return lastErr_; }

protected:
	void Init();
	Channel* FindChannel(string ch_name);
	static void FillAudio(void* udata, unsigned char* stream, int len);
	bool NeedMix(string ch_name, int len);
	bool IsSilent(unsigned char* src, int len);

protected:
	int inited_; 
	string	lastErr_;
	SDL_AudioDeviceID deviceID_;
	SDLAudioOpt_t option_;
    SDL_AudioSpec sdlAudioSpec_;
    
	eCurrentPlayState state_ = eStop;

	std::unordered_map<string, Channel*> channelsMap_;
	std::mutex mutex_;
};