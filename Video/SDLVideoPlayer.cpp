#include "SDLVideoPlayer.h"
#include "../Com/Common.h"
#include <Windows.h>



SDLVideoPlayer::SDLVideoPlayer(): lastErr_("") {
	winSpec_ = { "Video Player", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 960, 540,  SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN };

	Init();
}

SDLVideoPlayer::~SDLVideoPlayer() {
	Release();
}

int SDLVideoPlayer::Init() {
	// init SDL
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);

	return 0;
}

void SDLVideoPlayer::Release() {
	// quit SDL
	SDL_Quit();
}

int SDLVideoPlayer::Create() {
	// create window
	sdlWin_ = SDL_CreateWindow(winSpec_.name_, winSpec_.posx_, winSpec_.posy_, winSpec_.width_, winSpec_.height_, winSpec_.flags_);
	if (!sdlWin_) {
		lastErr_ = SDL_GetError();
		printf("[SDLVideoPlayer] could not create sdl window - exiting:%s\n", lastErr_.c_str());
		return -1;
	}

	// create renderer
	sdlRenderer_ = SDL_CreateRenderer(sdlWin_, -1, 0);
	// create texture
	sdlTexture_ = SDL_CreateTexture(sdlRenderer_, pixelSpec_.fmt_, SDL_TEXTUREACCESS_STREAMING, pixelSpec_.width_, pixelSpec_.height_);

	if (!eventThread_) {
		eventThread_ = new std::thread([&]() {
			while (!bExit_) {
				int ret = OnEvent();
				Sleep(2);
			}
		});
	}

	return 0;
}

void SDLVideoPlayer::Destroy() {
	// relese resource
	SDL_DestroyTexture(sdlTexture_);
	SDL_DestroyRenderer(sdlRenderer_);
	SDL_DestroyWindow(sdlWin_);

	SAFE_DEL(eventThread_);
}

int SDLVideoPlayer::OnEvent() {
	// process event
	int ret = -1;
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT) {
			bExit_ = 1;
			ret = 0;
		}
		else if (event.type == SDL_WINDOWEVENT) {
			switch (event.window.event) {
				case SDL_WINDOWEVENT_RESIZED: 
				case SDL_WINDOWEVENT_SIZE_CHANGED: {
					// if Resize
					SDL_GetWindowSize(sdlWin_, &winSpec_.width_, &winSpec_.height_);
					//SDL_SetWindowSize(sdlWin_, event.window.data1, event.window.data2);
					break;
				}
				case SDL_WINDOWEVENT_MOVED: {
					// if moved
					SDL_GetWindowPosition(sdlWin_, &winSpec_.posx_, &winSpec_.posy_);
					break;
				}
				/*case SDL_WINDOWEVENT_HIDDEN: {
					SDL_HideWindow(sdlWin_);
					break;
				}
				case SDL_WINDOWEVENT_SHOWN: {
					SDL_ShowWindow(sdlWin_);
					break;
				}
				case SDL_WINDOWEVENT_EXPOSED: {
					SDL_RaiseWindow(sdlWin_);
					break;
				}
				case SDL_WINDOWEVENT_MINIMIZED: {
					SDL_MinimizeWindow(sdlWin_);
					break;
				}
				case SDL_WINDOWEVENT_MAXIMIZED: {
					SDL_MaximizeWindow(sdlWin_);
					break;
				}
				case SDL_WINDOWEVENT_RESTORED: {
					SDL_RestoreWindow(sdlWin_);
					break;
				}*/
				default:
					break;
			}		
			
			ret = 0;
		}
		else
			ret = -1;
	}

	return ret;
}

int SDLVideoPlayer::Open(pixelSpec_t pixel_spec) {
	pixelSpec_ = pixel_spec;
	winSpec_.width_ = pixelSpec_.width_;
	winSpec_.height_ = pixelSpec_.height_;

	int ret = Create();

	return ret;
}

int SDLVideoPlayer::Show(const uint8_t *data, int size) {
	//uint8_t *y_plane = data;
	//uint8_t *u_plane = y_plane + pixelSpec_.width_ * pixelSpec_.height_;
	//uint8_t *v_plane = u_plane + (pixelSpec_.width_ * pixelSpec_.height_) >> 2;

	// update texture
	int ret = SDL_UpdateTexture(sdlTexture_, NULL, data, pixelSpec_.width_);
	if (ret != 0) {
		lastErr_ = SDL_GetError();
		printf("[SDLVideoPlayer] could not update sdl YUV texture - exiting:%s\n", lastErr_.c_str());
		return ret;
	}
	//FIX: If window is resize
	SDL_Rect sdl_rect;
	sdl_rect.x = 0;
	sdl_rect.y = 0;
	sdl_rect.w = winSpec_.width_;
	sdl_rect.h = winSpec_.height_;
	// clear render
	SDL_RenderClear(sdlRenderer_);
	// update renderer
	ret = SDL_RenderCopy(sdlRenderer_, sdlTexture_, NULL, &sdl_rect);
	if (ret != 0) {
		lastErr_ = SDL_GetError();
		printf("[SDLVideoPlayer] could not update sdl renderer - exiting:%s\n", lastErr_.c_str());
		return ret;
	}
	SDL_RenderPresent(sdlRenderer_);

	return 0;
}

void SDLVideoPlayer::Close() {
	bExit_ = 1;
	if (eventThread_->joinable())
		eventThread_->join();

	Destroy();
}

void SDLVideoPlayer::Delay(uint32_t ms) {
	SDL_Delay(ms);
}