#include "timer.hpp"
#include <vector>


Timer::Timer()
{
	start = std::chrono::high_resolution_clock::now();
}

double Timer::restart()
{
	auto now = std::chrono::high_resolution_clock::now();
	double diff = std::chrono::duration_cast<std::chrono::nanoseconds>(now - start).count();
	start = std::chrono::high_resolution_clock::now();
	paused = false;
	return diff / 1000000000.0;
}

double Timer::elapsed()
{
	auto now = std::chrono::high_resolution_clock::now();
	if (paused)
		now = pause_time;
	double diff = std::chrono::duration_cast<std::chrono::nanoseconds>(now - start).count();
	return diff / 1000000000.0;
}

void Timer::pause()
{
	if (!paused)
	{
		pause_time = std::chrono::high_resolution_clock::now();
		paused = true;
	}
}

void Timer::resume()
{
	if (paused)
	{
		auto now = std::chrono::high_resolution_clock::now();
		start += now - pause_time;
		paused = false;
	}
}

/*
void TimerGroup::add(Timer* t)
{
	
}

void TimerGroup::pauseAll()
{
}

void TimerGroup::resumeAll()
{
}

void TimerGroup::restartAll()
{
}
*/