#ifndef _MANAGER_HPP
#define _MANAGER_HPP

#include <algorithm>
#include <iostream>
#include <set>
#include <utility>
#include <vector>



class Task
{
	private:

		class Activity
		{
			public:
				void *(*routine)(void *);		///< routine
				void *direct_args;				///< direct arguments (not coming from dependencies)
				std::vector<std::pair<bool, void*> > funnel_args; /**< arguments coming from deps results.
																	   (port allocated, pointer to arg)*/
				int n_unresolved;				///< no. of yet unresolved dependencies
				std::set<std::pair<int, int> > dependent_ops;	/**< activities directly depending on this
																	 (op index, ret value port)*/

				Activity(void *(*func)(void *), void *arg);
				~Activity();
			
		};

		std::vector<Activity> activities;	// dependencies of the activies in the task

	public:
		Task();
		~Task();
		int add_activity(void *(*func)(void *), void *arg);
		int add_dependency(int a1, int a2);
		int link_ret_to_arg(int src, int dst, unsigned port);
};



class Manager
{
	private:

	public:
		const unsigned n_threads;

		Manager(unsigned pool_size);
		~Manager();
		void add_task(Task& t);
};

#endif