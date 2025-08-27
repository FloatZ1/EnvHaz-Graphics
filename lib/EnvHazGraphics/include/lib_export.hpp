
#pragma once

#if defined _WIN32 || defined __CYGWIN__
#ifdef BUILDING_EHAZGAPI
#define eHazGAPI __declspec(dllexport)
#else
#define eHazGAPI __declspec(dllimport)
#endif
#elif defined(__GNUC__) && __GNUC__ >= 4
#define eHazGAPI __attribute__((visibility("default")))
#else
#define eHazGAPI
#endif
