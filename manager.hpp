#ifndef _MANAGER_HPP
#define _MANAGER_HPP

#include <algorithm>
#include <iostream>
#include <map>
#include <queue>
#include <utility>
#include <vector>



class Task
{
	private:

		class Activity
		{
			public:
				void *(*routine)(void *);			///< routine
				void *direct_args;					///< direct arguments (not coming from dependencies)
				std::vector<std::pair<bool, void*> > funnel_args; /**< arguments coming from deps results.
																	   (port allocated, pointer to arg)*/
				int n_unresolved;					///< no. of yet unresolved dependencies
				std::map<int, int> dependent_ops;	/**< activities directly depending on this
														 (op index, ret value port) */

				Activity(void *(*func)(void *), void *arg);
				~Activity();
		};

		enum Color {WHITE, GREY, BLACK};					/**< used for DAG verification:
																 WHITE: node not explored yet 
																 GREY: node currently being explored
																 BLACK: node explored and all its descendants */

		std::vector<Activity> activities;					///< all the activities of the task
		std::priority_queue<std::pair<int, int> > ready_q;	/**< ops ready to execute (all deps satisfied)
																 (op index, priority) */


		void init_ready_q();
		int schedule();
		void complete_activity(unsigned id, void *retvalue);
		bool DFS_traverse(int a, std::vector<Color>& colors);

		void run_activity(int id);

	public:
		Task();
		~Task();
		int add_activity(void *(*func)(void *), void *arg);
		int add_dependency(int src, int dst);
		int link_ret_to_arg(int src, int dst, unsigned port);

		bool is_DAG();
		friend std::ostream& operator<<(std::ostream& os, const Task& t);
		friend std::ostream& operator<<(std::ostream& os, const Activity& a);
		friend class Manager;
};



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