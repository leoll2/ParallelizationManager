#include "parallelizer.hpp"
#include "debug.hpp"


Manager::Manager(unsigned pool_size) : 
	task(nullptr),
	n_threads(pool_size)
{

	/* TODO: allocate a pool of threads*/
}


Manager::~Manager()
{

	/* TODO: kill threads of the pool */
}


void Manager::add_task(Task& t)
{

	task = &t;
}


void* Manager::run_task()
{
	task->init_ready_q();

	Activity *current;
	while ((current = task->schedule())) {
		task->run_activity(*current);
	}

	return nullptr;
}