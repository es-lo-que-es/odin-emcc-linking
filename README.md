<h2>Odin to Emscripten linking</h2>

<p><i>this repo is a conclusion to my little reserach on linking 2 separate wasm modules generated with different compilers (in this case one generated with Emscripten and another with Odin)</i></p>

> NOTE: this concludes to rather a lazy solution. there might be a better way to do it but it is what it is.
>> Also most of my knwoledge about wasm was gained 2 days prior writing this so i am in no means expert

<p> <i>You can check out the end-result <a href="https://es-lo-que-es.github.io/odin-emcc-linking-build/">here</a> </i></p>

<hr/>

<h2> Introduction </h2>

<p> the reason behind this repo was my desire to use sdl2 from odin-built wasm module therefore i will use simple sdl2 based app as an example </p>

<p> we will start from something plain and simple: </p>

> NOTE: in real app u would probably want to free resources and stay away from 'panic' but for readability reasons i'll try to keep the following snippet as compact as possible

```Odin
package main
import "core:fmt"
import "vendor:sdl2"

main :: proc()
{
   width: i32: 400;
   height: i32: 400;

   err := sdl2.Init({.VIDEO});
   if err != 0 do panic("sdl init failed");
      
   window := sdl2.CreateWindow(nil, 0, 0, width, height, { .SHOWN });
   if window == nil do panic("create window failed");
   
   renderer := sdl2.CreateRenderer(window, -1, {.ACCELERATED});
   if renderer == nil do panic("create renderer failed");

   // fill created window with a red color
   sdl2.SetRenderDrawColor(renderer, 240, 120, 120, 255);
   sdl2.RenderClear(renderer);
   sdl2.RenderPresent(renderer);

   // delay for a native target so it doesnt close imidiately
   when ODIN_ARCH != .wasm32 do sdl2.Delay(3000); 
}
```
<p> minimal HTML page to run our wasm module </p>

```HTML
<!-- odin runtime for js_wasm target so u can do memory allocations and stuff -->
<!-- u can grab it at Odin/vendor/wasm/js/runtime.js -->
<script src="runtime.js"> </script>

<!-- script to run our wasm -->
<script> 

   odin.runWasm("main.wasm")
      .then( () => { console.log("success, hooray!"); } )
      .catch( (err) => { console.log("err: ", err); } );

</script>
```
<p> our page will error out with:</p> 

```Javascript
'WebAssembly.instantiate(): Import #1 module="system:SDL2": module is not an object or function'
```
<p> which basically means that our wasm module was looking for its imports and havent found them.</p>
<p> good thing we can provide theese imports during wasm module initialisation. lets look which functions do we need to provide </p>

> there are multiple ways to check wasm module import section. one of which to cast it to "human-readable"(not really) .wat format with tools like wasm2wat

<p> inspecting .wat file made from our module we can see the following imports: </p>

```
  (import "odin_env" "write" (func $odin_env..write (type 1)))
  (import "system:SDL2" "SDL_Init" (func $system:SDL2..SDL_Init (type 2)))
  (import "system:SDL2" "SDL_CreateWindow" (func $system:SDL2..SDL_CreateWindow (type 3)))
  (import "system:SDL2" "SDL_CreateRenderer" (func $system:SDL2..SDL_CreateRenderer (type 4)))
  (import "system:SDL2" "SDL_SetRenderDrawColor" (func $system:SDL2..SDL_SetRenderDrawColor (type 5)))
  (import "system:SDL2" "SDL_RenderClear" (func $system:SDL2..SDL_RenderClear (type 2)))
  (import "system:SDL2" "SDL_RenderPresent" (func $system:SDL2..SDL_RenderPresent (type 6)))
```

<p> lets modify our js script by adding a simple imports stub </p>

```HTML
<script src="runtime.js"> </script>

<script> 
   const sdl2_imports_stub = {

      "system:SDL2": {

         SDL_Init: (flags) => { console.log("sdl init was called"); },

         SDL_CreateWindow: (title, x, y, w, h, flags) => { console.log("create window was called"); },
         SDL_CreateRenderer: (pwindow, index, flags) => { console.log("create renderer was called"); },

         SDL_SetRenderDrawColor: (prenderer, r, g, b, a) => { console.log("set renderer color was called"); },
         SDL_RenderPresent: (prenderer) => { console.log("render present was called"); },
         SDL_RenderClear: (prenderer) => { console.log("render clear was called"); },
      }
   };

   odin.runWasm("main.wasm", {}, sdl2_imports_stub)
      .then( () => { console.log("success, hooray!"); } )
      .catch( (err) => { console.log("err: ", err); } );

</script>
```
<p> running the page will give us following output: </p>

```
index.html:8 sdl init was called
index.html:10 create window was called
runtime.js:1222 /path/odin-emcc-linking/main.odin(16:24) panic: create window failed
index.html:21 err:  RuntimeError: unreachable
    at runtime.default_assertion_failure_proc (879e2d8a:0x7e7)
    at runtime.panic (879e2d8a:0x9ee)
    at main.main (879e2d8a:0xdee)
    at _start (879e2d8a:0x1081)
    at Object.runWasm (runtime.js:1676:10)
```

<p> which means <b>"hooray! our module works"</b> but panics after failing to create a window since our stub-imports obviously dont really do anything :) </p>

<hr/>

<h2> Making it work! (linking) </h2>

<p> Since our wasm modules can not only import but also export functions we can use emscripten to compile a C module which would export all of the needed sdl2 stuff </p>
<p> u can find source's to my silly exports module in the 'exports/' folder as well as the CMake script to build it. it basically just re-exports some functions from sdl2 </p>

<p> now when we have our 2 modules we can "link" them together via imports object. lets add emscripten-built module (with all of its "magic" a.k.a runtime) and a canvas element to draw on to the page</p>

```HTML
<canvas style="display:block" id = "canvas" oncontextmenu="event.preventDefault()" tabindex=-1></canvas>

<script> 
   // emscripten module instance that will be initialised when runtime loads
   var Module = { canvas: document.getElementById("canvas") };
</script>

<!-- emscripten generated runtime for our exports module -->
<script src="sdl_exports.js"> </script>
<script src="runtime.js"> </script>

<script> 

   const sdl2_imports = {

      "system:SDL2": {
         SDL_Init: (flags) => Module._Init(flags),
         SDL_Quit: () => Module._Quit(),

         SDL_CreateWindow: (title, x, y, w, h, flags) => Module._CreateWindow(title, x, y, w, h, flags),
         SDL_CreateRenderer: (pwindow, index, flags) => Module._CreateRenderer(pwindow, index, flags),

         SDL_SetRenderDrawColor: (prenderer, r, g, b, a) => Module._SetRenderDrawColor(prenderer, r, g, b, a),
         SDL_RenderPresent: (prenderer) => Module._RenderPresent(prenderer),
         SDL_RenderClear: (prenderer) => Module._RenderClear(prenderer),
      }
   };

   // wait for emscripten runtime to be initialised
   Module.onRuntimeInitialized = () => {
      odin.runWasm("main.wasm", {}, sdl2_imports)
         .then( () => { console.log("success, hooray!"); } )
         .catch( (err) => { console.log("err: ", err); } );
   }

</script>
```
<p> and <b>TA-DAAA!</b> we can see our beautiful red rectangle on the screen</p>

![photo_2024-01-10_15-41-38](https://github.com/es-lo-que-es/odin-emcc-linking/assets/143192493/19853153-f56d-4033-9760-298e7b572040)


<hr/>

<h2> COMPLICATIONS </h2>

<p><i>u may have already noticed a cute lil elephant in the room.</i></p>
<p> You have 2 wasm modules. And each has its own memory. Its own address-space if u will. </p>

<p> Which means dereferencing any pointer passed from one module to another will be meaningless and perhaps unsafe.</p>

<p> window and renderer pointers in our program point to objects in sdl_exports module memory, but since they are only used by calls from its own module - everything works just fine.</p>

<p> yet passing window title to <b>sdl2.CreateWindow</b> wont work since <b>cstring (const char*)</b> we pass lives in memory of odin built module and when sdl_exports module recieves that <b>const char *</b> it will search its own memory for that address(and God only knows what it will find there :) ). Its basically a "same address - different street" issue if u want to simplify it.</p>

<hr/>

<h2> Solutions </h2>

<ul>
   <li> <p><b>OPTION 1 ( the lamest ):</b> Avoid Pointers </p>
      <p> Since the problem lies in pointers not relying on them would be a reasonable solution.</p>
      <p> Sadly wasm only supports basic types for its imports/exports parameters so passing by copy wouldnt work even for something as simple as <b>SDL_Rect</b>. Therefore the only way of avoiding pointers i see is to re-structure ur app in a way that doesnt rely on passing pointers between modules which is cringe</p>

   <li> <p><b>OPTION 2 ( not as lame but still sucks ):</b> Mod Runtime & Copy Stuff Around</p>

   <p> We can implement our own silly 'pass by copy' for pointer parameters meaning pre-alloc some kind of copy buffer in each module and than make js functions that will copy parameters between modules and change pointers to address of a copy.</p> 

> NOTE: it is important to keep in mind that there are not only input but also output parameters like <i>event</i> in <b>SDL_PollEvent</b> which will require additional handling (since we need to copy them back after the call)

>> also <i> beware of the shallow copy </i>. good thing i dont need to deep-copy anything for my sdl project

<li> <p> <b>OPTION 3:</b> Make Your Own Runtime And Share Memory </p>
<p> in theory u could pass <b>WebAssembly.Memory</b> object with importsObject while u init the module. but it would require you to handle alot of stuff and i havent really looked into it so dont quote me on that.</p>

</ul>

<h2> Making it work! (part 2) </h2>

<p> second option has its perfomance cost but doesnt sound that hard to implement. so lets make our odin program a bit more complicated and see if we can deal with it </p>

```Odin
// global renderer
renderer: ^sdl2.Renderer;

// we will export a function that polls through events and fills a rect when we click our window(canvas)
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
```

<p> lets begin by adding copy buffer to our emscripten-built module and functions to access it from js runtime </p>

```C
#define COPY_BUFFER_SIZE 256
uint8_t COPY_BUFFER_ARR[COPY_BUFFER_SIZE];

void * get_copy_buffer() { return COPY_BUFFER_ARR; }
int copy_buffer_size() { return COPY_BUFFER_SIZE; }
```

<p> now we can make our CopyBuffer class in Javascript to pass pointers around </p>

``` Javascript
   class CopyBuffer {

      offset = 0; // ring buffer offset

      static RECT_SIZE = 16;
      static EVENT_SIZE = 56;
      static MAX_STRING_LEN = 32;
      
      // wasm modules memory is implemented as SharedArrayBuffer
      init(ibuffer, iptr, isize, obuffer) 
      {
         // offset and size of our copy buffer
         this.iptr = iptr; this.size = isize;

         // to read/write those array buffers we will need to create Uint8Array view;  
         this.iview = new Uint8Array(ibuffer);
         this.oview = new Uint8Array(obuffer);
      }
      
      // function to copy parameter into our copy buffer
      copy(ptr, sizeof, out=false) 
      {
         // loop ring buffer
         if ( this.offset + sizeof > this.size ) this.offset = 0;

         let copy_ptr = this.iptr + this.offset;
         // copy our data only if its input parameter since output-par should be filled by callee
         if ( !out ) this.iview.set(this.oview.subarray(ptr, ptr+sizeof), copy_ptr);

         this.offset += sizeof;

         return copy_ptr;
      }

      copy_back(ptr, copy_ptr, sizeof) 
      {
         this.oview.set(this.iview.subarray(copy_ptr, copy_ptr+sizeof), ptr);
      }
      
      oevent(ptr) { return this.copy(ptr, CopyBuffer.EVENT_SIZE, true); }

      istring(ptr) { return this.copy(ptr, CopyBuffer.MAX_STRING_LEN); }
      irect(ptr) { return this.copy(ptr, CopyBuffer.RECT_SIZE); }
   }
```

<p> we are almost done! since odin.runWasm initialises and runs wasm module in one go we wont be able to access module's memory. so lets create our own function to init module and return exports (reusing odin.runWasm code) </p>

```Javascript
async function getOdinExports(wasmPath, consoleElement, extraForeignImports) {
{
   let wasmMemoryInterface = new WasmMemoryInterface();
   let imports = odinSetupDefaultImports(wasmMemoryInterface, consoleElement);

   if (extraForeignImports !== undefined) imports = { ...imports, ...extraForeignImports };

   const response = await fetch(wasmPath);
   const file = await response.arrayBuffer();
   const wasm = await WebAssembly.instantiate(file, imports);

   exports = wasm.instance.exports;
   wasmMemoryInterface.setExports(exports);
   wasmMemoryInterface.setMemory(exports.memory);

   return exports;
}
```

<p> now lets add this all to our web page and see if it even works :) </p>

```Javascript
   let cpbuff = new CopyBuffer();

   const sdl2_imports = {

      "system:SDL2": {
         SDL_Init: (flags) => Module._Init(flags),
         SDL_Quit: () => Module._Quit(),

         SDL_CreateWindow: (title, x, y, w, h, flags) => Module._CreateWindow(cpbuff.istring(title), x, y, w, h, flags),
         SDL_CreateRenderer: (pwindow, index, flags) => Module._CreateRenderer(pwindow, index, flags),

         SDL_RenderFillRect: (prenderer, prect) => Module._RenderFillRect(prenderer, cpbuff.irect(prect)),

         SDL_SetRenderDrawColor: (prenderer, r, g, b, a) => Module._SetRenderDrawColor(prenderer, r, g, b, a),
         SDL_RenderPresent: (prenderer) => Module._RenderPresent(prenderer),
         SDL_RenderClear: (prenderer) => Module._RenderClear(prenderer),

         // aditional handling for output parameter
         SDL_PollEvent: (pevent) => {

            let copy = cpbuff.oevent(pevent);  
            let result = Module._PollEvent(copy);
            
            cpbuff.copy_back(pevent, copy, CopyBuffer.EVENT_SIZE);

            return result;
         }
      }
   };

   // i put main code inside async func cus js callbacks are cringe af
   async function run_app() {
      
      let exports = await getOdinExports("main.wasm", {}, sdl2_imports);

      let offs = Module._get_copy_buffer();
      let size = Module._copy_buffer_size();
      cpbuff.init(Module.HEAP8.buffer, offs, size, exports.memory.buffer);
      
      exports._start();
      
      function animate() {
         if ( exports._iterate() ) requestAnimationFrame(animate);
         else exports._end();
      }
         
      requestAnimationFrame(animate)
   }

   // wait for emscripten runtime to be initialised
   Module.onRuntimeInitialized = () => { run_app(); }
```

<p> and <b>TA-DAAA(again)</b> everything seems to be working as expected! </p>



https://github.com/es-lo-que-es/odin-emcc-linking/assets/143192493/89ade4d8-7c85-4324-bd4a-fe5d4c9e1658



<p> we even got our window title set by <b>sdl2.CreateWindow</b> :) </p>

```Odin
   window := sdl2.CreateWindow("Cool Snapy Title", 0, 0, width, height, { .SHOWN });
```
