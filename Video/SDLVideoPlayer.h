#pragma once

#include <string>
#include <stdint.h>
#include <thread>
#include <mutex>

#ifdef HPLAYER_EXPORTS
#define HPLAYER_API __declspec(dllexport)
#else
#define HPLAYER_API __declspec(dllimport)
#endif


#include <SDL.h>

#pragma comment(lib, "SDL2.lib")

using std::string;

typedef struct winSpec {
	char	name_[256];
	int		posx_;
	int		posy_;
	int		width_;
	int		height_;
	int		flags_;
}winSpec_t, *pWinSpec_t;

typedef struct pixelSpec {
	// IYUV: Y + U + V  (3 planes)
	// YV12: Y + V + U  (3 planes)
	// I420Ò²½ÐIYUV, Ò²½ÐYUV420
	uint32_t	fmt_;
	int			width_;
	int			height_;
}pixelSpec_t, *pPixelSpec_t;


class HPLAYER_API SDLVideoPlayer {
public:
	SDLVideoPlayer();
	~SDLVideoPlayer();

	//return 0 on success, or -1 on error
	int Open(pixelSpec_t pixel_spec = { SDL_PIXELFORMAT_IYUV, 960, 540 });
	//return 0 on success, or -1 on error
	int Show(const uint8_t *data, int size);
	void Close();
	// wait a specified number of milliseconds before returning.
	void Delay(uint32_t ms);

	string GetLastErr() { return lastErr_; }

protected:
	int Init();
	void Release();

	int Create();
	void Destroy();

	int OnEvent();

protected:
	winSpec_t		winSpec_;
	pixelSpec_t		pixelSpec_;

	SDL_Window		*sdlWin_{ NULL };
	SDL_Renderer	*sdlRenderer_{ NULL };
	SDL_Texture		*sdlTexture_{ NULL };
	string			lastErr_;

	std::thread*	eventThread_{ NULL };
	std::mutex		eventMtx_;
	int				bExit_{ 0 };
};
