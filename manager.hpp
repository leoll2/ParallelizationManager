#ifndef _MANAGER_HPP
#define _MANAGER_HPP

#include <algorithm>
#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <utility>
#include <vector>


enum class Color {WHITE, GREY, BLACK};	/**< used for DAG verification:
											 WHITE: node not explored yet 
											 GREY: node currently being explored
											 BLACK: node explored and all its descendants */

class Activity
{
	private:
		static int ID_gen;				///< ID generator
		int id;							///< ID (for debug purposes only)
		unsigned (*routine)(const std::vector<void*>&);	///< routine
		std::vector<std::pair<bool, void*> > params; 	/**< routine arguments.
															 The first is passed directly, others come from dep results
															 (port allocated, pointer to arg) */
		std::map<Activity*, int> dependent_ops;			/**< activities directly depending on this
															 (operation, ret value port) */
		int n_unresolved;				///< no. of yet unresolved dependencies (updated during execution)
		Color col;						///< used for DAG verification

	public:
		Activity(unsigned (*func)(const std::vector<void*>&), void *arg);
		~Activity();
		friend std::ostream& operator<<(std::ostream& os, const Activity& a);
		friend class Task;
};


class ComparePrio
{
public:
    bool operator()(std::pair<Activity*, int> p1, std::pair<Activity*, int> p2) {
        return p1.second > p2.second;
    }
};


class Task
{
	private:

		std::vector<Activity*> activities;			///< all the activities of the task
		std::priority_queue<std::pair<Activity*, int>, std::vector<std::pair<Activity*, int>>, ComparePrio> ready_q;	
													/**< ops ready to execute (all deps satisfied)
														 (operation, priority) */
		void init_ready_q();
		Activity* schedule();
		void complete_activity(Activity& a, void *retvalue);
		bool DFS_traverse(Activity& a);

		void run_activity(Activity& a);

	public:
		Task();
		~Task();
		void add_activity(Activity& a);
		int add_dependency(Activity& a_src, Activity& a_dst);
		int link_ret_to_arg(Activity& a_src, Activity& a_dst, unsigned port);
		bool is_DAG();
		friend std::ostream& operator<<(std::ostream& os, const Task& t);
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