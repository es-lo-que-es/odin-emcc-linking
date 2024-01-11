package main

import "core:fmt"
import "core:time"
import "vendor:sdl2"
import "core:runtime"
import "core:math/rand"

renderer: ^sdl2.Renderer;

@(export, link_name="_iterate")
iterate :: proc "contextless" (ctx: runtime.Context) -> bool
{
   context = ctx;

   event: sdl2.Event;
   for sdl2.PollEvent(&event) {

      #partial switch event.type {
         case .MOUSEBUTTONDOWN: fill_random_rect({event.motion.x, event.motion.y});
         case .QUIT: return false;
      }
   }
   sdl2.RenderPresent(renderer);
   return true;
}


rand_u8 :: proc() -> u8
{  
   return u8(rand.int_max(256));
}


fill_random_rect :: proc(point: sdl2.Point)
{
   side := 30 + i32(rand.int_max(40));
   rect: sdl2.Rect = { point.x - side/2, point.y - side/2, side, side };

   fmt.printf("random rectangle around %v; %v\n", point.x, point.y);

   sdl2.SetRenderDrawColor(renderer, 240, 240, 240, 255);
   sdl2.RenderClear(renderer);
   sdl2.SetRenderDrawColor(renderer, rand_u8(), rand_u8(), rand_u8(), 255);

   sdl2.RenderFillRect(renderer, &rect);
}


main :: proc()
{
   width: i32: 400; height: i32: 400;
   rand.set_global_seed(u64(time.now()._nsec));

   err := sdl2.Init({.VIDEO});
   if err != 0 do panic("sdl init failed");
      
   window := sdl2.CreateWindow("Cool Snapy Title", 0, 0, width, height, { .SHOWN });
   if window == nil do panic("create window failed");
   
   renderer = sdl2.CreateRenderer(window, -1, {.ACCELERATED});
   if renderer == nil do panic("create renderer failed");

   sdl2.SetRenderDrawColor(renderer, 240, 240, 240, 255);
   sdl2.RenderClear(renderer);

   // main loop for native target
   when ODIN_ARCH != .wasm32 {
      for iterate() do sdl2.Delay(100); 
      
      sdl2.DestroyRenderer(renderer);
      sdl2.DestroyWindow(window);
      sdl2.Quit();
   }
}
