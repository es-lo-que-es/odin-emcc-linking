#ifndef SDL_EXPORTS
#define SDL_EXPORTS

#include "SDL2/SDL.h"

/* SDL2 EXPORTS */
int Init(uint32_t flags);
void Quit(void);


SDL_Window * CreateWindow(const char *title, int x, int y, int w, int h, Uint32 flags);
void DestroyWindow(SDL_Window * window);


SDL_Renderer * CreateRenderer(SDL_Window * window, int index, Uint32 flags);
void DestroyRenderer(SDL_Renderer * renderer);


int SetRenderDrawColor(SDL_Renderer * renderer, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
void RenderPresent(SDL_Renderer * renderer);
int RenderClear(SDL_Renderer * renderer);


int RenderFillRect(SDL_Renderer * renderer, const SDL_Rect * rect);
int PollEvent(SDL_Event * event);


void * get_copy_buffer(void);
int copy_buffer_size(void);

#endif
