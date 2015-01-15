#ifndef __liquid_cpp__core__defines__
#define __liquid_cpp__core__defines__

#if defined(__APPLE__) || defined(__linux__)

  #define PLATFORM_UNIX

  #include <stddef.h>

#elif defined(_WIN32)

  #define PLATFORM_WINDOWS

#endif

#endif