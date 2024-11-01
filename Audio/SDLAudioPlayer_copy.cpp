#include "stdafx.h"
#include "SDLAudioPlayer_copy.h"
#include <iostream>




SDLNonQueueAudioPlayer::SDLNonQueueAudioPlayer(SDLAudioOpt_t sdl_audio_opt):
	inited_(0),	lastErr_(""), option_(sdl_audio_opt)
{
	Init();
}

SDLNonQueueAudioPlayer::~SDLNonQueueAudioPlayer() {
	RemoveAllChannels();
	SDL_Quit();
}

void SDLNonQueueAudioPlayer::Init() {
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
        const char* err = SDL_GetError();
		lastErr_ = *err;
		return;
    }

    sdlAudioSpec_.freq = option_.freq;
    sdlAudioSpec_.format = static_cast<SDL_AudioFormat >(option_.format);
    sdlAudioSpec_.channels = static_cast<Uint8 >(option_.channels);
    sdlAudioSpec_.silence = 0;
    sdlAudioSpec_.samples = static_cast<Uint16>(option_.samples);
    sdlAudioSpec_.userdata = this;
    sdlAudioSpec_.callback = (SDL_AudioCallback)FillAudio;

	inited_ = 1;
    return;
}

void SDLNonQueueAudioPlayer::FillAudio(void *udata, unsigned char* stream, int len) {
	// TODO: handle silent volume;
	/*auto *channels_map_ptr = static_cast<std::unordered_map<std::string, Channel*> *>(udata);
	int channel_count = 0;
	for (auto it = channels_map_ptr->begin(); it != channels_map_ptr->end(); ++it) {
	if (rbuf_used(it->second->buffer) == 0) {
	continue;
	}
	if (need_mix(it->second->buffer, len)) {
	channel_count++;
	}
	}*/

	SDL_memset(stream, 0, len);
	unsigned char* src = new unsigned char[len];

	SDLNonQueueAudioPlayer* p_sdl_audio_player = static_cast<SDLNonQueueAudioPlayer*>(udata);

	std::lock_guard<std::mutex> locker(p_sdl_audio_player->mutex_);

	for (auto it = p_sdl_audio_player->channelsMap_.begin(); it != p_sdl_audio_player->channelsMap_.end(); ++it) {
		if (src) {
			memset(src, 0, sizeof(unsigned char)*len);

			if (p_sdl_audio_player->ChannelEmpty(it->first)) {
				continue;
			}

			if (p_sdl_audio_player->NeedMix(it->first, len)) {
				int nsize = it->second->Read(src, len);
				SDL_MixAudioFormat(
					stream,
					static_cast<const Uint8 *>(src),
					AUDIO_S16LSB,
					static_cast<Uint32 >(nsize),
					SDL_MIX_MAXVOLUME
				);
			}
		}
	}

	delete[] src;
}

int SDLNonQueueAudioPlayer::Start() {
	if (!inited_)
		return -1;

    if (state_ == ePlaying) { return 0; }
    if (state_ != eStop) { return -1; }
    if (int err_code = SDL_OpenAudio(&sdlAudioSpec_, NULL) < 0) {
		const char* err = SDL_GetError();
		lastErr_ = *err;

        SDL_Log("[SDLAudioPlayer] Audio Device Open failed %d: %s", err_code, err);
        return -1;
    }
	
    SDL_PauseAudio(0);
    state_ = ePlaying;

    return 0;
}

int SDLNonQueueAudioPlayer::NewChannel(string ch_name) {
	if (!inited_)
		return -1;

	std::lock_guard<std::mutex> locker(mutex_);
    auto got = channelsMap_.find(ch_name);
    if (got == channelsMap_.end()) {
        Channel* channel = new Channel(ch_name);
        channelsMap_[ch_name] = channel;
        return 0;
    } else {
		lastErr_ = "[SDLAudioPlayer] Channel already exist:" + ch_name;
        return -1;
    }
}

int SDLNonQueueAudioPlayer::RemoveChannel(string ch_name) {
	if (!inited_)
		return -1;

	std::lock_guard<std::mutex> locker(mutex_);
    auto got = channelsMap_.find(ch_name);
    if (got == channelsMap_.end()) {
		lastErr_ = "[SDLAudioPlayer] Channel not found:" + ch_name;
        return -1;
    } else {
        delete got->second;
        channelsMap_.erase(ch_name);
        return 0;
    }
}

int SDLNonQueueAudioPlayer::RemoveAllChannels() {
	if (!inited_)
		return -1;

	std::lock_guard<std::mutex> locker(mutex_);
	for (auto iter = channelsMap_.begin(); iter != channelsMap_.end(); iter++) {
		delete iter->second;
		//iter = channels_map.erase(iter);
	}
	channelsMap_.clear();
}

void SDLNonQueueAudioPlayer::Clean(string ch_name) {
	if (!inited_)
		return;

	std::lock_guard<std::mutex> locker(mutex_);
    Channel* ch = FindChannel(ch_name);
    if (ch != nullptr) {
        ch->Clean();
    }
}

void SDLNonQueueAudioPlayer::CleanAll() {
	if (!inited_)
		return;

	std::lock_guard<std::mutex> locker(mutex_);
    for (auto iter = channelsMap_.begin(); iter != channelsMap_.end(); iter++) {
		iter->second->Clean();
    }
}

int SDLNonQueueAudioPlayer::Write(void* buf, int len, string ch_name) {
	if (!inited_)
		return -1;

	std::lock_guard<std::mutex> locker(mutex_);
    Channel* ch = FindChannel(ch_name);
    if (ch != nullptr) {
		while (ch->Avail() < len) {
			return -2;
		}

		return ch->Write(buf, len);
    }

	lastErr_ = "[SDLAudioPlayer] Channel not found:" + ch_name;
	return -3;
}

int SDLNonQueueAudioPlayer::Stop() {
	if (!inited_)
		return -1;

    if (state_ == eStop) { return 0; }
    CleanAll();
	SDL_CloseAudio();
    state_ = eStop;
    return 0;
}

int SDLNonQueueAudioPlayer::Pause() {
	if (!inited_)
		return -1;

    if (state_ == ePause) { return 0; }
    if (state_ != ePlaying) { 
		lastErr_ = "[SDLAudioPlayer] status is not playing.";
		return -1;
	}

    SDL_PauseAudio(1);
    state_ = ePause;
    return 0;
}

int SDLNonQueueAudioPlayer::Resume() {
	if (!inited_)
		return -1;

    if (state_ == ePlaying) { return 0; }
    if (state_ != ePause) { 
		lastErr_ = "[SDLAudioPlayer] status is not pause.";
		return -1;
	}
    
	SDL_PauseAudio(0);
    state_ = ePlaying;
    return 0;
}

Channel* SDLNonQueueAudioPlayer::FindChannel(string ch_name) {
	if (!inited_)
		return nullptr;

	auto got = channelsMap_.find(ch_name);
	if (got == channelsMap_.end()) {
		return nullptr;
	}
	else {
		return got->second;
	}
}

bool SDLNonQueueAudioPlayer::ChannelEmpty(string ch_name) {
	return FindChannel(ch_name)->Empty();
}

bool SDLNonQueueAudioPlayer::ChannelFull(string ch_name) {
	return FindChannel(ch_name)->Full();
}

bool SDLNonQueueAudioPlayer::NeedMix(string ch_name, int len) {
	bool ret = FindChannel(ch_name)->Used() >= len;
	return ret;
}

bool SDLNonQueueAudioPlayer::IsSilent(unsigned char* src, int len) {
	for (int i = 0; i < len; i++) {
		if (src[i] != 0) {
			return false;
		}
	}
	return true;
}