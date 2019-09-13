#pragma once

#include <chrono>
#include <vector>

class Timer
{
	bool paused = false;
	std::chrono::time_point<std::chrono::high_resolution_clock> start;
	std::chrono::time_point<std::chrono::high_resolution_clock> pause_time;
public:
	Timer();
	double restart();
	double elapsed();
	void pause();
	void resume();
};


/*
** TODO: implement if needed, 
** problem with deleted timers
class TimerGroup
{
	std::vector<Timer*> timers;
public:
	void add(Timer*);
	void pauseAll();
	void resumeAll();
	void restartAll();
};
*/