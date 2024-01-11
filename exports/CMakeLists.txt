cmake_minimum_required(VERSION 3.10)

project(sdl2_exports)


SET(CMAKE_C_COMPILER "emcc")

set(USE_FLAGS "-sUSE_SDL=2")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${USE_FLAGS}")

set(
   EXPORTED_FUNCTIONS
   "_Init,_Quit,_CreateWindow,_DestroyWindow,_CreateRenderer,_DestroyRenderer,_SetRenderDrawColor,_RenderClear,_RenderPresent,_RenderFillRect,_PollEvent,_get_copy_buffer,_copy_buffer_size"
)

set(LINKER_FLAGS "-sEXPORTED_RUNTIME_METHODS=ccall,cwrap -sEXPORTED_FUNCTIONS=${EXPORTED_FUNCTIONS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${USE_FLAGS} ${LINKER_FLAGS} ${PRELOAD_FILES}")
set(CMAKE_EXECUTABLE_SUFFIX .js)

add_executable(sdl_exports exports.c)
