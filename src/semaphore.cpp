#include "semaphore.hpp"


Semaphore::Semaphore(unsigned cnt) :
	count(cnt)
{}


/* Set the semaphore count to the specified value */
void Semaphore::set(unsigned cnt)
{
	std::unique_lock<std::mutex> lock(mtx);
	
	count = cnt;
}


void Semaphore::wait()
{
	std::unique_lock<std::mutex> lock(mtx);

	while(!count)
		cond.wait(lock);
	--count;
}


/* Never sleeps for more than rel_time after initial call or being woken up. */
bool Semaphore::wait_limited(const std::chrono::milliseconds& rel_time)
{
	std::cv_status ret;
	std::unique_lock<std::mutex> lock(mtx);

	while(!count) {
		ret = cond.wait_for(lock, rel_time);
		if (ret == std::cv_status::timeout)
			return false;
	}
	--count;
	return true;
}


void Semaphore::signal()
{
	std::lock_guard<std::mutex> lock(mtx);
	
	++count;
	cond.notify_one();
}
