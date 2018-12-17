#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <utility>
#include <vector>

#include "parallelizer.hpp"
#include "debug.hpp"


std::atomic_int PManager::Worker::ID_gen = ATOMIC_VAR_INIT(0);

/* Routine of a worker: wait to be requested by manager, then execute the  */
void PManager::Worker::routine()
{
	while(true) {
		std::unique_lock<std::mutex> lock(ready_mtx);

		// Wait for manager to put something in cur_act and give authorization to run (signaling)
		ready_to_go.wait(lock);		// this is also signaled when shutting down the worker
		// If the worker is asked to stop, this is the safest moment (no activity in progress)
		if (should_stop)
			break;

		D(std::cout << "I'm the worker #" << id << std::endl;)
		// Execute the code of the activity (here is the core of the routine)
		cur_task->run_activity(*cur_act);

		// After finishing, notify the master about the worker being available again
		master->employable_worker(this);
	}
}


/* Worker constructor */
PManager::Worker::Worker(PManager *owner) :
	cur_act(nullptr),
	cur_task(nullptr),
	master(owner),
	id(ID_gen++),
	should_stop(ATOMIC_VAR_INIT(false))
{
	thread = new std::thread(&PManager::Worker::routine, this);
	master->employable_worker(this);	// worker is initialized as available
}


/* Worker destructor */
PManager::Worker::~Worker()
{
	delete thread;
}


/* Join worker's thread */
void PManager::Worker::join(void)
{
	this->thread->join();
}


/* Manager constructor */
PManager::PManager(unsigned pool_size) : 
	n_workers(pool_size)
{
	// Allocate a pool of threads
	pool.reserve(n_workers);
	for (unsigned i = 1; i <= n_workers; ++i)
		pool.push_back(new Worker(this));
}


/* Manager destructor */
PManager::~PManager()
{
	// Signal workers to stop asap
	for (auto& w : pool) {
		w->should_stop = true;
		w->ready_to_go.notify_one();
	}

	// Join the workers, then deallocate them
	for (auto& w : pool) {
		w->join();
		delete w;
	}
	pool.clear();
}


/* Assign a task to the manager */
void PManager::add_task(Task& t)
{
	runqueue.push_back(&t);	 // FIFO insert in the runqueue
}


/* Returns true if there is any available worker, false otherwise. */
bool PManager::any_worker_available() 
{
	return !avail_workers.empty();
}


/* Pick a worker among the set of available ones */
PManager::Worker* PManager::hire_worker()
{
	std::unique_lock<std::mutex> lock(worker_avail_mtx);
	worker_avail.wait(lock, std::bind(&PManager::any_worker_available, this));
	Worker *wp = *avail_workers.begin();	// first elem of the set
	avail_workers.erase(avail_workers.begin());
	return wp;	
}


/* Put a worker in the set of available ones */
void PManager::employable_worker(Worker *w)
{
	std::unique_lock<std::mutex> lock(worker_avail_mtx);
	avail_workers.insert(w);
	worker_avail.notify_one();
}


/* Pick an activity that is ready to be executed. The scheduling policy follows.
*  Consider tasks in FIFO order: if the first task has any activity ready for execution
*  (i.e. all dependencies are satisfied), then choose one of those; otherwise, move to
*  the next task and do the same, and so on. If no activity is ready for execution yet,
*  likely because dependencies are currently being solved, simply return nullptr. 
*/
Activity* PManager::schedule_activity() {

	Activity *scheduled_act;

	// Iterate through all non-finished tasks in FIFO order
	std::list<Task*>::iterator it = runqueue.begin();
	while (it != runqueue.end()) {
		// Try to schedule any activity from this task
		scheduled_act = (*it)->schedule();
		// If found, return it
		if (scheduled_act) {
			D(std::cout << "Got an activity to schedule." << std::endl;)
			return scheduled_act;
		} else {  // Move to the next task
			// But not before checking if task was finished (if so, remove from runqueue)
			if ((*it)->completed)
				it = runqueue.erase(it);
			else
				it++;
		}
	}
	D(std::cout << "Found nothing to schedule." << std::endl;)
	return nullptr;
}


/* Routine of the manager. After initializing the ready queues of each task, 
*  loop through all activities until all jobs are completed.
*/
void PManager::run()
{
	Activity *scheduled_act;

	// For every task, setup its ready queue
	for (auto const& t : runqueue)
		t->init_ready_q();

	// As long as there are unfinished tasks
	while (runqueue.size()) {
		// Try to schedule the next activity
		scheduled_act = schedule_activity();
		// If any is available, find a worker to which the activity ought to be assigned
		if (scheduled_act) {
			Worker *w = hire_worker();
			w->cur_act = scheduled_act;
			w->cur_task = scheduled_act->owner;
			w->ready_to_go.notify_one();
			std::cout << "Activity " << scheduled_act->id << " assigned to worker " << w->id << std::endl;
		} else {  // otherwise, if no activity available, wait for one to become available
			// TODO
			D(std::cout << "Waiting for activities to finish..." << std::endl;)
			//std::this_thread::sleep_for (std::chrono::seconds(1));	// TODO: replace with better solution
		}
	}
}