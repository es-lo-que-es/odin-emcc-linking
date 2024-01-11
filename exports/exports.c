#include "exports.h"


#define COPY_BUFFER_SIZE 256
uint8_t COPY_BUFFER_ARR[COPY_BUFFER_SIZE];

void * get_copy_buffer() { return COPY_BUFFER_ARR; }
int copy_buffer_size() { return COPY_BUFFER_SIZE; }


/* SDL2 EXPORTS */
int Init(uint32_t flags) 
{ 
   return SDL_Init(flags);
}


void Quit(void) { SDL_Quit(); }


SDL_Window * CreateWindow(const char *title, int x, int y, int w, int h, Uint32 flags)
{ 
   return SDL_CreateWindow(title, x, y, w, h, flags);
}


void DestroyWindow(SDL_Window * window) { SDL_DestroyWindow(window); }


SDL_Renderer * CreateRenderer(SDL_Window * window, int index, Uint32 flags)
{ 
   return SDL_CreateRenderer(window, index, flags);
}


void DestroyRenderer(SDL_Renderer * renderer)
{
   SDL_DestroyRenderer(renderer);
}


int PollEvent(SDL_Event * event)
{
   return SDL_PollEvent(event);
}


int RenderFillRect(SDL_Renderer * renderer, const SDL_Rect * rect)
{
   return SDL_RenderFillRect(renderer, rect);
}


int SetRenderDrawColor(SDL_Renderer * renderer, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{ 
   return SDL_SetRenderDrawColor(renderer, r, g, b, a);
}


void RenderPresent(SDL_Renderer * renderer) 
{ 
   SDL_RenderPresent(renderer);
}


int RenderClear(SDL_Renderer * renderer) 
{ 
   return SDL_RenderClear(renderer);
}
