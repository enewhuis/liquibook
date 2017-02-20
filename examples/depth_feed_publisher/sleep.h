#pragma once
#ifdef _WIN32
#include <Windows.h>
static const unsigned int MILLISECONDS_PER_SECOND = 1000;
inline void sleep(unsigned int seconds)
{
    ::Sleep(MILLISECONDS_PER_SECOND * seconds);
}
#else
#include <unistd.h>
#endif