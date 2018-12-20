#ifndef _TASK_HPP
#define _TASK_HPP

#include <mutex>
#include <queue>
#include <utility>
#include <vector>

#include "activity.hpp"

class Activity;

class ActivityPrioCompare
{
	public:
	    bool operator()(std::pair<Activity*, int> p1, std::pair<Activity*, int> p2);
};


class Task
{
	private:

		std::vector<Activity*> activities;			// all the activities of the task
		std::priority_queue<std::pair<Activity*, int>, std::vector<std::pair<Activity*, int>>, ActivityPrioCompare> ready_q;	
													/* operations ready to execute (all deps satisfied)
													   (operation, priority) */
		std::mutex ready_q_mtx;						// mutex to protect ready_q
		bool has_endpoint;							// used to verify that there's one and only one endpoint activity
		bool completed;								// true when all the activities are finished
		void **result;								// pointer to buffer to store the final result of the task

		void init_ready_q();
		Activity* schedule();
		void complete_activity(Activity& a, void *retvalue, unsigned retsize);
		bool DFS_traverse(Activity& a) const;
		void run_activity(Activity& a);

	public:
		Task(void*& res);
		~Task();
		void add_activity(Activity& a);
		int add_dependency(Activity& a_src, Activity& a_dst);
		int link_ret_to_arg(Activity& a_src, Activity& a_dst, unsigned port);
		bool is_DAG() const;
		friend std::ostream& operator<<(std::ostream& os, const Task& t);
		friend class PManager;
};


#endif