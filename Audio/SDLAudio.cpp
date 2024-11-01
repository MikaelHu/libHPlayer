#include "SDLAudio.h"



SDLAudio::SDLAudio() {
	Init();
}

SDLAudio::~SDLAudio() {
	SDL_Quit();
}

void SDLAudio::Init() {
	if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
		const char* err = SDL_GetError();
		lastErr_ = *err;
		SDL_Log("[SDLAudio] Init failed : %s", err);
		return;
	}
}

int SDLAudio::LoadWav(const char *file, SDLAudioOpt_t *src_opt, Uint8 **audio_buf, Uint32 *audio_len) {
	SDL_AudioSpec spec;
	if (SDL_LoadWAV(file, &spec, audio_buf, audio_len) == NULL) {
		const char* err = SDL_GetError();
		lastErr_ = *err;

		SDL_Log("[SDLAudio] LoadWav file failed : %s",  err);
		return -1;
	}

	src_opt->freq = spec.freq;
	src_opt->format = spec.format;
	src_opt->channels = spec.channels;
	src_opt->samples = spec.samples; // total samples = sample * channels * samplesize.

	return 0;
}

void SDLAudio::FreeWav(Uint8 * audio_buf) {
	SDL_FreeWAV(audio_buf);
}

int SDLAudio::NewStream(const SDLAudioOpt_t src_opt, const SDLAudioOpt_t dst_opt) {
	SDL_AudioStream *stream = SDL_NewAudioStream(src_opt.format, src_opt.channels, src_opt.freq, dst_opt.format, dst_opt.channels, dst_opt.freq);
	if (!stream) {
		const char* err = SDL_GetError();
		lastErr_ = *err;

		SDL_Log("[SDLAudio] NewStream failed : %s", err);
		return -1;
	}

	um_stream_.emplace(streamIDCounter_, stream);
	return streamIDCounter_++;
}

int SDLAudio::StreamPut(int stream_id, const void *buf, int len) {
	UM_AUDIO_STREAM::iterator iter = um_stream_.find(stream_id);
	if (iter != um_stream_.end()){
		int err_code = SDL_AudioStreamPut(iter->second, buf, len);
		if (err_code < 0) {
			const char* err = SDL_GetError();
			lastErr_ = *err;

			SDL_Log("[SDLAudio] StreamPut failed %d: %s", err_code, err);
		}
		return err_code;
	}
	return -1;
}

int SDLAudio::StreamGet(int stream_id, void *buf, int len) {
	UM_AUDIO_STREAM::iterator iter = um_stream_.find(stream_id);
	if (iter != um_stream_.end()) {
		int err_code = SDL_AudioStreamGet(iter->second, buf, len);
		if (err_code < 0) {
			const char* err = SDL_GetError();
			lastErr_ = *err;

			SDL_Log("[SDLAudio] StreamGet failed %d: %s", err_code, err);
		}
		return err_code;
	}
	return -1;
}

int SDLAudio::StreamAvail(int stream_id) {
	UM_AUDIO_STREAM::iterator iter = um_stream_.find(stream_id);
	if (iter != um_stream_.end()) {
		int err_code = SDL_AudioStreamAvailable(iter->second);
		if (err_code < 0) {
			const char* err = SDL_GetError();
			lastErr_ = *err;

			SDL_Log("[SDLAudio] SteamAvail failed %d: %s", err_code, err);
		}
		return err_code;
	}
	return -1;
}

int SDLAudio::StreamFlush(int stream_id) {
	UM_AUDIO_STREAM::iterator iter = um_stream_.find(stream_id);
	if (iter != um_stream_.end()) {
		int err_code = SDL_AudioStreamFlush(iter->second);
		if (err_code < 0) {
			const char* err = SDL_GetError();
			lastErr_ = *err;

			SDL_Log("[SDLAudio] SteamFlush failed %d: %s", err_code, err);
		}
		return err_code;
	}
	return -1;
}

void SDLAudio::StreamClear(int stream_id) {
	UM_AUDIO_STREAM::iterator iter = um_stream_.find(stream_id);
	if (iter != um_stream_.end()) {
		SDL_AudioStreamClear(iter->second);
	}
}

void SDLAudio::FreeStream(int stream_id) {
	UM_AUDIO_STREAM::iterator iter = um_stream_.find(stream_id);
	if (iter != um_stream_.end()) {
		SDL_FreeAudioStream(iter->second);
	}
}