#include "semaphore.hpp"


Semaphore::Semaphore(unsigned cnt) :
	count(cnt)
{}


/* Set the semaphore counter to the specified value. */
void Semaphore::set(unsigned cnt)
{
	std::unique_lock<std::mutex> lock(mtx);
	
	count = cnt;
}


/* Sleep if the semaphore counter is 0. */
void Semaphore::wait()
{
	std::unique_lock<std::mutex> lock(mtx);

	while(!count)
		cond.wait(lock);
	--count;
}


/* Sleeps for at most rel_time after initial call or after being woken up. */
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


/* Increment the semaphore counter and wake up blocked tasks, if any. */
void Semaphore::signal()
{
	std::lock_guard<std::mutex> lock(mtx);
	
	++count;
	cond.notify_one();
}
