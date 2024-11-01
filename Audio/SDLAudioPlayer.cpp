#include "SDLAudioPlayer.h"
#include "Channel.h"
#include "../Core/hlring/rbuf.h"



SDLAudioPlayer::SDLAudioPlayer(SDLAudioOpt_t audio_opt):
	inited_(0), lastErr_(""), audioOption_(audio_opt) {

}

SDLAudioPlayer::~SDLAudioPlayer() {
}

void SDLAudioPlayer::Delay(Uint32 ms) {
	SDL_Delay(ms);
}

int SDLAudioPlayer::GetNumAudioDevices(int iscapture) {
	return SDL_GetNumAudioDevices(iscapture);
}

const char* SDLAudioPlayer::GetAudioDeviceName(int index, int iscapture) {
	return SDL_GetAudioDeviceName(index, iscapture);
}




SDLQueueAudioPlayer::SDLQueueAudioPlayer(SDLAudioOpt_t audio_opt): SDLAudioPlayer(audio_opt)
{
	Init();
}

SDLQueueAudioPlayer::~SDLQueueAudioPlayer() {
	SDL_Quit();
}

void SDLQueueAudioPlayer::Init() {
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
        const char* err = SDL_GetError();
		lastErr_ = *err;
		SDL_Log("[SDLQueueAudioPlayer] Init failed : %s", err);
		return;
    }

    audioSpec_.freq = audioOption_.freq;
    audioSpec_.format = static_cast<SDL_AudioFormat >(audioOption_.format);
    audioSpec_.channels = static_cast<Uint8 >(audioOption_.channels);
    audioSpec_.silence = 0;
    audioSpec_.samples = static_cast<Uint16>(audioOption_.samples);
    audioSpec_.userdata = nullptr;
    audioSpec_.callback = nullptr;

	inited_ = 1;
}

int SDLQueueAudioPlayer::Start() {
	if (!inited_)
		return -1;

    if (state_ == ePlaying) { return 0; }
    if (state_ != eStop) { return -1; }

	// 获取可用扬声器列表，不需要可以忽略
	int deviceCount = SDL_GetNumAudioDevices(0);
	SDL_Log("[SDLQueueAudioPlayer] SDL_GetNumAudioDevices %d\n", deviceCount);
	for (int i = 0; i < deviceCount; i++) {
		SDL_Log("[SDLQueueAudioPlayer] Audio Device %d: %s\n", i, SDL_GetAudioDeviceName(i, 0));
	}

	// 指定扬声器，不需要第一个参数可以填nullptr
	deviceID_ = SDL_OpenAudioDevice(SDL_GetAudioDeviceName(1, 0), 0, &audioSpec_, nullptr, 0/*SDL_AUDIO_ALLOW_ANY_CHANGE*/);
	SDL_Log("[SDLQueueAudioPlayer] device_id %d\n", deviceID_);
	if (deviceID_ == 0) {
		const char* err = SDL_GetError();
		lastErr_ = *err;

		SDL_Log("[SDLQueueAudioPlayer] Audio Device Open failed: %s", err);
		return -1;
	}

	SDL_PauseAudioDevice(deviceID_, 0);
    state_ = ePlaying;

    return 0;
}

int SDLQueueAudioPlayer::Write(void* buf, int len) {
	if (!inited_)
		return -1;

	//SDL feed data
	return SDL_QueueAudio(deviceID_, buf, len);
}

void SDLQueueAudioPlayer::Clean() {
	SDL_ClearQueuedAudio(deviceID_);
}

Uint32 SDLQueueAudioPlayer::GetQueuedAudioSize() {
	return SDL_GetQueuedAudioSize(deviceID_);
}

Uint32 SDLQueueAudioPlayer::DequeueAudio(void *data, Uint32 len) {
	return SDL_DequeueAudio(deviceID_, data, len);
}

int SDLQueueAudioPlayer::Stop() {
	if (!inited_)
		return -1;

    if (state_ == eStop) { return 0; }
	SDL_CloseAudioDevice(deviceID_);
    state_ = eStop;
    return 0;
}

int SDLQueueAudioPlayer::Pause() {
	if (!inited_)
		return -1;

    if (state_ == ePause) { return 0; }
    if (state_ != ePlaying) { 
		lastErr_ = "[SDLQueueAudioPlayer] status is not playing.";
		return -1;
	}

	SDL_PauseAudioDevice(deviceID_, 1);
    state_ = ePause;
    return 0;
}

int SDLQueueAudioPlayer::Resume() {
	if (!inited_)
		return -1;

    if (state_ == ePlaying) { return 0; }
    if (state_ != ePause) { 
		lastErr_ = "[SDLQueueAudioPlayer] status is not pause.";
		return -1;
	}
    
	SDL_PauseAudioDevice(deviceID_, 0);
    state_ = ePlaying;
    return 0;
}

SDLAudioPlayer::eCurrentPlayState SDLQueueAudioPlayer::Status() {
	int status = SDL_GetAudioDeviceStatus(deviceID_);
	return (eCurrentPlayState)status;
}



SDLNonQueueAudioPlayer::SDLNonQueueAudioPlayer(SDLAudioOpt_t audio_opt) : SDLAudioPlayer(audio_opt)
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

	audioSpec_.freq = audioOption_.freq;
	audioSpec_.format = static_cast<SDL_AudioFormat >(audioOption_.format);
	audioSpec_.channels = static_cast<Uint8 >(audioOption_.channels);
	audioSpec_.silence = 0;
	audioSpec_.samples = static_cast<Uint16>(audioOption_.samples);
	audioSpec_.userdata = this;
	audioSpec_.callback = (SDL_AudioCallback)FeedAudio;

	inited_ = 1;
	return;
}

void SDLNonQueueAudioPlayer::FeedAudio(void *udata, unsigned char* stream, int len) {
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
	if (int err_code = SDL_OpenAudio(&audioSpec_, NULL) < 0) {
		const char* err = SDL_GetError();
		lastErr_ = *err;

		SDL_Log("[SDLNonQueueAudioPlayer] Audio Device Open failed %d: %s", err_code, err);
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
	}
	else {
		lastErr_ = "[SDLNonQueueAudioPlayer] Channel already exist:" + ch_name;
		return -1;
	}
}

int SDLNonQueueAudioPlayer::RemoveChannel(string ch_name) {
	if (!inited_)
		return -1;

	std::lock_guard<std::mutex> locker(mutex_);
	auto got = channelsMap_.find(ch_name);
	if (got == channelsMap_.end()) {
		lastErr_ = "[SDLNonQueueAudioPlayer] Channel not found:" + ch_name;
		return -1;
	}
	else {
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

		SDL_LockAudio();
		int nret = ch->Write(buf, len);
		SDL_UnlockAudio();

		return nret;
	}

	lastErr_ = "[SDLNonQueueAudioPlayer] Channel not found:" + ch_name;
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
		lastErr_ = "[SDLNonQueueAudioPlayer] status is not playing.";
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
		lastErr_ = "[SDLNonQueueAudioPlayer] status is not pause.";
		return -1;
	}

	SDL_PauseAudio(0);
	state_ = ePlaying;
	return 0;
}

SDLAudioPlayer::eCurrentPlayState SDLNonQueueAudioPlayer::Status() {
	//SDL_OpenAudio(), always acts on device ID 1.
	int status = SDL_GetAudioDeviceStatus(1/*deviceID_*/);
	return (eCurrentPlayState)status;

	//return state_;
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