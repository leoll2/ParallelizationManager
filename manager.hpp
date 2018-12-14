#ifndef _MANAGER_HPP
#define _MANAGER_HPP

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <utility>
#include <vector>


#ifdef DEBUG 
#define D(x) x
#else 
#define D(x)
#endif

#define NO_RETURN() \
	*_retbuf = nullptr; \
	return 0;

#define RETURN(arg) \
	unsigned _retsize = sizeof arg; \
	*_retbuf = malloc(_retsize); \
	std::memcpy(*_retbuf, (void*)&arg, _retsize); \
	return _retsize;


/* Handy macro to specify activity code in a compact fashion. */
#define ACTIVITY(name) \
    class name : public Activity { \
		public: \
			name(void *direct_arg, bool endpoint = false) : Activity(direct_arg, endpoint) {} \
			virtual unsigned operator() (const std::vector<void*>& args, void **_retbuf); \
    }; \
    unsigned name::operator() (const std::vector<void*>& args, void **_retbuf)


#define GET_ARG(name, type, port) \
    type name = *(type*)args[(port)];


#define RETRIEVE_RESULT(dst, src, type) \
    dst = *((type*)(src));



enum class Color {WHITE, GREY, BLACK};	/**< used for DAG verification:
											 WHITE: node not explored yet 
											 GREY: node currently being explored
											 BLACK: node explored and all its descendants */

class Activity
{
	private:
		static int ID_gen;				///< ID generator
		int id;							///< ID (for debug purposes only)
		std::vector<std::pair<bool, void*> > params; 	/**< routine arguments.
															 The first is passed directly, others come from dep results
															 (port allocated, pointer to arg) */
		std::map<Activity*, int> dependent_ops;			/**< activities directly depending on this
															 (operation, ret value port) */
		int n_unresolved;				///< no. of yet unresolved dependencies (updated during execution)
		bool is_endpoint;				///< is it the final task?
		void ** final_res;				///< if final task, pointer to buffer to store the result
		Color col;						///< used for DAG verification

	public:
		Activity(void *direct_arg, bool endpoint = false);
		~Activity();

		virtual unsigned operator() (const std::vector<void *>& args, void **_retbuf) = 0;

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
		bool has_endpoint;							///< used to verify that there's one and only one endpoint activity
		void **result;								///< pointer to buffer to store the final result of the task

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