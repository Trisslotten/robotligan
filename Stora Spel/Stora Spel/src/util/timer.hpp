#ifndef TIMER_HPP_
#define TIMER_HPP_

#include <chrono>
#include <vector>

class Timer
{
	bool paused_ = false;
	std::chrono::time_point<std::chrono::high_resolution_clock> start_;
	std::chrono::time_point<std::chrono::high_resolution_clock> pause_time_;
public:
	Timer();
	double Restart();
	double Elapsed();
	void Pause();
	void Resume();
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

#endif //TIMER_HPP_