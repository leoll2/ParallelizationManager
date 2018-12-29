#ifndef _PARALLELIZER_HPP
#define _PARALLELIZER_HPP

#include <atomic>
#include <mutex>
#include <condition_variable>
#include <list>
#include <set>
#include <thread>

#include "activity.hpp"
#include "debug.hpp"
#include "semaphore.hpp"
#include "task.hpp"


class PManager
{
	private:
		class Worker
		{	
			private:
				Activity *cur_act;          // activity currently being processed
				std::thread *thread;        // thread of this worker
				PManager *const master;     // owner of the worker
			public:
				static std::atomic_int ID_gen;  // worker ID generator
				const int id;                   // worker unique identifier
				std::atomic_bool should_stop;   // the worker's thread should terminate
				Semaphore data_avail;           // signaled by manager after filling cur_act and cur_task

				Worker(PManager *owner);
				~Worker();
				void routine();
				void stop();
				void join();
				friend class PManager;
		};

		std::list<Task*> runqueue;              // list of not yet finished tasks (managed FIFO)
		std::vector<Worker*> pool;              // pool of worker threads
		std::set<Worker*> avail_workers;        // set of available (not busy) workers
		std::mutex avail_workers_mtx;           // mutex to protect available workers pool
		std::condition_variable worker_avail;   // signaled when a worker becomes available
		Semaphore act_finished;                 // signaled when a worker finishes a task
		
		bool any_worker_available();
		Worker* hire_worker();
		void free_worker(Worker *w);
		Activity* schedule_activity();

	public:
		const unsigned n_workers;               // number of workers

		PManager(unsigned pool_size);
		~PManager();
		void add_task(Task& t);
		void run();
};

#endif
