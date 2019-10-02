#include "timer.hpp"
#include <vector>


Timer::Timer()
{
	start_ = std::chrono::high_resolution_clock::now();
}

double Timer::Restart()
{
	auto now = std::chrono::high_resolution_clock::now();
	double diff = std::chrono::duration_cast<std::chrono::nanoseconds>(now - start_).count();
	start_ = std::chrono::high_resolution_clock::now();
	paused_ = false;
	return diff / 1000000000.0;
}

double Timer::Elapsed()
{
	auto now = std::chrono::high_resolution_clock::now();
	if (paused_)
		now = pause_time_;
	double diff = std::chrono::duration_cast<std::chrono::nanoseconds>(now - start_).count();
	return diff / 1000000000.0;
}

void Timer::Pause()
{
	if (!paused_)
	{
		pause_time_ = std::chrono::high_resolution_clock::now();
		paused_ = true;
	}
}

void Timer::Resume()
{
	if (paused_)
	{
		auto now = std::chrono::high_resolution_clock::now();
		start_ += now - pause_time_;
		paused_ = false;
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