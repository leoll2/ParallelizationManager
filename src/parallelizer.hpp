#ifndef _PARALLELIZER_HPP
#define _PARALLELIZER_HPP

#include <atomic>
#include <mutex>
#include <condition_variable>
#include <list>
#include <set>
#include <thread>

#include "activity.hpp"
#include "task.hpp"


class PManager
{
	private:
		class Worker
		{	
			private:
				Activity *cur_act;			// activity currently being processed
				Task *cur_task;				// task owning the current activity
				std::thread *thread;		// thread of this worker
				PManager *const master;		// owner of the worker
			public:
				static std::atomic_int ID_gen;	// worker ID generator
				const int id;					// worker unique identifier
				std::atomic_bool should_stop;	// the worker's thread should terminate
    			std::condition_variable ready_to_go;	// signaled by manager when allowed to process the activity 
    			std::mutex ready_mtx;			// mutex to protect ready_to_go

				Worker(PManager *owner);
				~Worker();
				void routine();
				void stop();
				void join();
				friend class PManager;
		};

		std::list<Task*> runqueue;				// list of not yet finished tasks (managed FIFO)
		std::vector<Worker*> pool;				// pool of worker threads
		std::set<Worker*> avail_workers;		// set of available (not busy) workers
		std::condition_variable worker_avail;	// signaled when a worker becomes available
		std::mutex worker_avail_mtx;			// mutex to protect worker_avail

		bool any_worker_available();
		Worker* hire_worker();
		void employable_worker(Worker *w);
		Activity* schedule_activity();

	public:
		const unsigned n_workers;		// number of workers

		PManager(unsigned pool_size);
		~PManager();
		void add_task(Task& t);
		void run();
};

#endif