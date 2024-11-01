#pragma once

#ifdef HPLAYER_EXPORTS
#define HPLAYER_API __declspec(dllexport)
#else
#define HPLAYER_API __declspec(dllimport)
#endif

#include <unordered_map>
#include <string>
#include <SDL.h>

#pragma comment(lib, "SDL2.lib")

using std::unordered_map;
using std::string;

typedef struct SDLAudioOpt {
	int freq;
	int channels;
	int samples;
	int format;
}SDLAudioOpt_t, *pSDLAudioOpt_t;


class HPLAYER_API SDLAudio {
	typedef unordered_map<int, SDL_AudioStream *> UM_AUDIO_STREAM;
public:
	SDLAudio();
	~SDLAudio();

	//This function loads a WAV from a file.
	//LoadWav("sample.wav", ...);
	//return src_opt: the audio spec; audio_buf: audio data; audio_len: audio data len.
	//return 0 on success, or -1 on error.
	int LoadWav(const char *file, SDLAudioOpt_t *src_opt, Uint8 **audio_buf, Uint32 *audio_len);
	//This function frees audio data previously allocated with LoadWav().
	void FreeWav(Uint8 * audio_buf);

	//return id >= 0 success, -1 on error.
	int NewStream(const SDLAudioOpt_t src_opt, const SDLAudioOpt_t dst_opt);
	//return 0 on success, or -1 on error.
	int StreamPut(int stream_id, const void *buf, int len);
	//return The number of bytes read from the stream, or -1 on error
	int StreamGet(int stream_id, void *buf, int len);
	//return the number of converted/resampled bytes available. maybe zero. -1 on error
	int StreamAvail(int stream_id);
	//flush the stream. return -1 on error
	int StreamFlush(int stream_id);
	//clear the stream.
	void StreamClear(int stream_id);
	//free the stream.
	void FreeStream(int stream_id);

	string GetLastErr() { return lastErr_; }

protected:
	void Init();

protected:
	int					streamIDCounter_{ 0 };
	UM_AUDIO_STREAM		um_stream_;
	string				lastErr_{""};
};