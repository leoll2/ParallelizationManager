#ifndef _PARALLELIZER_HPP
#define _PARALLELIZER_HPP

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <utility>
#include <vector>

#include "activity.hpp"
#include "task.hpp"


class Manager
{
	private:
		Task* task;		// TODO: support for multiple tasks

	public:
		const unsigned n_threads;

		Manager(unsigned pool_size);
		~Manager();
		void add_task(Task& t);
		void* run_task();	// TODO: add support for multiple tasks
};

#endif