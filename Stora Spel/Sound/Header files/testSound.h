#ifndef TEST_SOUND_H
#define TEST_SOUND_H

#ifdef MAKEDLL
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __declspec(dllimport)
#endif

EXPORT int testSound();
__declspec(dllexport) int testSound();

#endif // !TEST_SOUND_H


