#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <utility>
#include <vector>

#include "parallelizer.hpp"


std::atomic_int PManager::Worker::ID_gen = ATOMIC_VAR_INIT(0);

/* Routine of a worker: wait to be requested by the manager, then execute the appropriate function */
void PManager::Worker::routine()
{
	while(true) {
		// Wait for manager to put something in cur_act, but periodically check if should stop!
		while (!data_avail.wait_limited(std::chrono::milliseconds(100))) {
			// If the worker is asked to stop, this is the safest moment (no activity in progress)
			if (should_stop)
				return;
  		}
		D(std::cout << "[W" << id << "]" << " starting activity " << cur_act->id << std::endl);
		
		// Execute the code of the activity (here is the core of the routine)
		cur_task->run_activity(*cur_act);

		// Signal the end of an activity
		master->act_finished.signal();

		D(std::cout << "[W" << id << "]" << " finished activity " << cur_act->id << std::endl);

		// After finishing, notify the master about the worker being available again
		master->free_worker(this);
	}
}


/* Worker constructor */
PManager::Worker::Worker(PManager *owner) :
	cur_act(nullptr),
	cur_task(nullptr),
	master(owner),
	id(ID_gen++),
	should_stop(ATOMIC_VAR_INIT(false)),
	data_avail(0)
{
	thread = new std::thread(&PManager::Worker::routine, this);
	master->free_worker(this);	// worker is initialized as available
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
	act_finished(0),
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
	for (auto& w : pool)
		w->should_stop = true;

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
	std::unique_lock<std::mutex> lock(avail_workers_mtx);

	while(avail_workers.empty())
		worker_avail.wait(lock);

	Worker *wp = *avail_workers.begin();	// first elem of the set
	avail_workers.erase(avail_workers.begin());

	return wp;	
}


/* Put a worker in the set of available ones */
void PManager::free_worker(Worker *w)
{
	std::unique_lock<std::mutex> lock(avail_workers_mtx);

	avail_workers.insert(w);
	worker_avail.notify_one();
}


/* Pick an activity that is ready to be executed. The scheduling policy follows.
*  Consider tasks in FIFO order: if the first task has any activity ready for execution
*  (i.e. all dependencies are satisfied), then choose one of those; otherwise, move to
*  the next task and do the same, and so on. If no activity is ready for execution yet,
*  likely because dependencies are currently being solved, simply return nullptr. */
Activity* PManager::schedule_activity() {

	Activity *scheduled_act;

	// Iterate through all non-finished tasks in FIFO order
	std::list<Task*>::iterator it = runqueue.begin();
	while (it != runqueue.end()) {
		// Try to schedule any activity from this task
		scheduled_act = (*it)->schedule();
		// If found, return it
		if (scheduled_act) {
			return scheduled_act;
		} else {  // Move to the next task
			// But not before checking if task was finished (if so, remove from runqueue)
			if ((*it)->completed)
				it = runqueue.erase(it);
			else
				it++;
		}
	}
	return nullptr;
}


/* Routine of the manager. After initializing the ready queues of each task, 
*  loop through all activities until all jobs are completed. */
void PManager::run()
{
	Activity *scheduled_act;

	// For every task, setup its ready queue
	for (auto const& t : runqueue)
		t->init_ready_q();

	// As long as there are unfinished tasks
	while (runqueue.size()) {
		// Detect if any worker notifies the end of an activity from now on
		act_finished.set(0);
		// Try to schedule the next activity
		scheduled_act = schedule_activity();
		// If any is available, find a worker to which the activity ought to be assigned
		if (scheduled_act) {
			Worker *w = hire_worker();
			w->cur_act = scheduled_act;
			w->cur_task = scheduled_act->owner;
			w->data_avail.signal();

			D(std::cout << "[M] Activity " << scheduled_act->id 
			            << " (task " << w->cur_task->id << ") assigned to worker " << w->id << std::endl);
		} else {
			/* Tasks are removed from runqueue only after being fully completed. 
			   Therefore, if runqueue is empty, it means that all tasks were finished. */
			if (!runqueue.size())
				return;
			/* If no activity is schedulable, wait for another to finish (signaled by worker)
			   before checking again. If a worker finished right after we had checked for available
			   activities, this semaphore is not 0 and the wait doesn't block indeed.
			   A simple condition_variable would not be enough to handle signal before wait. */
			D(std::cout << "[M] No activity to schedule yet, waiting for a worker to finish... " << std::endl);
			act_finished.wait();
		}
	}
}
