#ifndef TEST_H
#define TEST_H

#ifdef MAKEDLL
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __declspec(dllimport)
#endif

EXPORT int testGraphics();


#endif // !TEST_H


