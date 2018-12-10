#include "manager.hpp"


Task::Task()
{

	std::cout << "Created task" << std::endl;
}


Task::~Task()
{

	std::cout << "Deallocated task" << std::endl;
}


int Task::add_activity(void *(*func)(void *), void *arg)
{

	std::cout << "Added activity" << std::endl;
	return 0;
}


int Task::add_precedence(int a1, int a2)
{

	std::cout << "Added precedence constraint" << std::endl;
	return 0;
}


int Task::link_ret_to_arg(int src, int dst, unsigned port)
{

	std::cout << "Bind return to argument" << std::endl;
	return 0;
}


Manager::Manager(unsigned pool_size) : n_threads(pool_size)
{

	/* TODO: allocate a pool of threads*/

	std::cout << "Initialized manager" << std::endl;
}


Manager::~Manager()
{

	/* TODO: kill threads of the pool */
}


void Manager::add_task(Task& t) {

	/* TODO: aggiungi task */
}