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
				Activity *cur_act;				///< activity currently being executed
				Task *cur_task;					///< task containing the current activity
				std::thread *thread;
				PManager * const master;		///< owner of the worker
			public:
				static std::atomic_int ID_gen;	///< ID generator
				const int id;					///< ID (for debug purposes only)
				std::atomic_bool should_stop;
    			std::condition_variable ready_to_go;	///< the manager signals when the thread is initialized and ready to go
    			std::mutex ready_mtx;

				Worker(PManager *owner);
				~Worker();
				void routine();
				void stop();
				void join();
				friend class PManager;
		};

		std::list<Task*> runqueue;				// List of not yet finished tasks (FIFO)
		std::vector<Worker*> pool;				// Pool of worker threads
		std::set<Worker*> avail_workers;		// Set of available (not busy) workers
		std::condition_variable worker_avail;	// Is any worker available?
		std::mutex worker_avail_mtx;

		bool any_worker_available();
		Worker* hire_worker();
		void employable_worker(Worker *w);

	public:
		const unsigned n_workers;

		PManager(unsigned pool_size);
		~PManager();
		void add_task(Task& t);
		void run();
};

#endif