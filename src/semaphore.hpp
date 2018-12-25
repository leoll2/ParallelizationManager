#ifndef _SEMAPHORE_HPP
#define _SEMAPHORE_HPP

#include <mutex>
#include <condition_variable>


class Semaphore
{
	private:
		std::mutex mtx;
		std::condition_variable cond;
		unsigned count;

	public:
		Semaphore(unsigned cnt);
		void set(unsigned cnt);
		void wait();
		bool wait_limited(const std::chrono::milliseconds& rel_time);
		void signal();
};

#endif
