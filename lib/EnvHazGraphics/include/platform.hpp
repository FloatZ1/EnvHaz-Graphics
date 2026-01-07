#pragma once

#ifdef PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN

#define NOMINMAX
#include <windows.h>
#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <SDL3/SDL_log.h>





#elif defined(PLATFORM_LINUX)
#include <unistd.h>
#include <sys/stat.h>
#elif defined(PLATFORM_MAC)
#include <unistd.h>
#include <sys/stat.h>
#endif