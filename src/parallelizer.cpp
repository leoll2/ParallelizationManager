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


void PManager::Worker::routine()
{

	while(true) {
		std::unique_lock<std::mutex> lock(ready_mtx);
		ready_to_go.wait(lock);

		if (should_stop)
			break;

		std::cout << "I'm the worker #" << id << std::endl;
		cur_task->run_activity(*cur_act);
		master->employable_worker(this);
	}
}


PManager::Worker::Worker(PManager *owner) :
	cur_act(nullptr),
	cur_task(nullptr),
	master(owner),
	id(ID_gen++),
	should_stop(ATOMIC_VAR_INIT(false))
{
	thread = new std::thread(&PManager::Worker::routine, this);
	master->employable_worker(this);
}


PManager::Worker::~Worker()
{}


void PManager::Worker::join(void)
{
	this->thread->join();
}


PManager::PManager(unsigned pool_size) : 
	n_workers(pool_size)
{
	/* Allocate a pool of threads*/
	pool.reserve(n_workers);
	for (unsigned i = 1; i <= n_workers; ++i)
		pool.push_back(new Worker(this));

}


PManager::~PManager()
{
	// Signal workers to stop asap
	for (auto& w : pool) {
		w->should_stop = true;
		w->ready_to_go.notify_one();
	}

	// Join the worker threads
	for (auto& w : pool)
		w->join();
}


void PManager::add_task(Task& t)
{
	runqueue.push_back(&t);
}


bool PManager::any_worker_available() 
{
	return !avail_workers.empty();
}


PManager::Worker* PManager::hire_worker()
{

	std::unique_lock<std::mutex> lock(worker_avail_mtx);
	worker_avail.wait(lock, std::bind(&PManager::any_worker_available, this));
	Worker *wp = *avail_workers.begin();
	avail_workers.erase(avail_workers.begin());
	return wp;	
}


void PManager::employable_worker(Worker *w)
{
	std::unique_lock<std::mutex> lock(worker_avail_mtx);
	avail_workers.insert(w);
	worker_avail.notify_one();
}


void PManager::run()
{
	Activity *scheduled_act;
	Task *scheduled_task;

	// For every task, setup the order to execute its activities
	for (auto const& t : runqueue)
		t->init_ready_q();

	// As long as there are unfinished tasks
	while (runqueue.size()) {
		// Find the next activity to be scheduled
		std::list<Task*>::iterator it = runqueue.begin();
		while (it != runqueue.end()) {
			scheduled_act = (*it)->schedule();
			if (scheduled_act) {
				scheduled_task = *it;
				std::cout << "Got an activity to schedule" << std::endl;
				break;
			} else {
				std::cout << "Didn't find an activity!" << std::endl;
				// If the queue is empty and the task is over, remove it
				if ((*it)->completed)
					it = runqueue.erase(it);
				else
					it++;
			}
		}

		if (scheduled_act) {
			Worker *w = hire_worker();
			w->cur_act = scheduled_act;
			w->cur_task = scheduled_task;
			w->ready_to_go.notify_one();
		} else {
			// TODO: attendi il completamento di qualche attività prima di ricontrollare
			std::cout << "Waiting for activities to finish..." << std::endl;
			std::this_thread::sleep_for (std::chrono::seconds(1));	// TODO: improve
		}
	}
}