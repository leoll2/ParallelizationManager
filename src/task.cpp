#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>

#include "debug.hpp"
#include "task.hpp"


int Task::ID_gen = 0;


/* Comparison function for activities in a priority queue (like ready_q). */
bool ActivityPrioCompare::operator()(std::pair<Activity*, int> p1, std::pair<Activity*, int> p2)
{
	return p1.second > p2.second;
}


/* Initialize the ready queue of the task, inserting those activities which 
*  don't have dependencies at all, */
void Task::init_ready_q()
{
	std::unique_lock<std::mutex> lock(ready_q_mtx);

	for (auto const &a : activities) {
		if (a->n_unresolved == 0)
			ready_q.emplace(std::make_pair(a, a->dependent_ops.size()));
	}
	assert(!ready_q.empty() && "No task schedulable!");
}


/* Extract from the ready queue the first schedulable activity, if any. 
*  If none is available, return nullptr. */
Activity* Task::schedule()
{
	std::unique_lock<std::mutex> lock(ready_q_mtx);

	if (ready_q.empty())
		return nullptr;
	auto ret = ready_q.top();
	ready_q.pop();
	return ret.first;
}


/* Perform the final actions required after completing the execution on an activity. */
void Task::complete_activity(Activity& a, void *retvalue, unsigned retsize) 
{
	if (a.is_endpoint) {
		// Copy the result in the provided buffer
		*(a.final_res) = malloc(retsize);
		std::memcpy(*(a.final_res), retvalue, retsize);
		completed = true;	// from now on the external entities are allowed to read the result
	} else {
		// For each dependent activity a_next
		for (auto const &dep : a.dependent_ops) {
			Activity& a_next = *(dep.first);
			int port = dep.second;

			// Funnel the return value into the parameter list of a_next, in the appropriate port
			if (port > 0) {
				assert(a_next.params[port].first && "Attempt to write in a non-allocated port");
				if (retsize) {
					a_next.params[port].second = malloc(retsize);
					std::memcpy(a_next.params[port].second, retvalue, retsize);
				} else
					a_next.params[port].second = nullptr;
			}
			// Resolve dependency for successor activities. If none left, insert in ready queue.
			if (--a_next.n_unresolved == 0) {
				std::unique_lock<std::mutex> lock(ready_q_mtx);
				ready_q.emplace(std::make_pair(&a_next, a_next.dependent_ops.size()));
			}
		}
	}
	// Deallocate actual arguments (except the first, which is allocated externally)
	auto arg = ++std::begin(a.params);
	while (arg != std::end(a.params)) {
		if ((*arg).first && (*arg).second)
			free((*arg).second);
		++arg;
	}
	// Deallocate result temporary buffer
	if (retsize)
		free(retvalue);
}


/* Execute the specified activity */
void Task::run_activity(Activity& a) {

	void *retbuf;		// buffer to store the result
	unsigned retsize;	// length of the buffer

	// Set up the actual arguments
	std::vector<void*> args;
	args.reserve(a.params.size());
	for (auto const &arg : a.params)
		args.push_back(arg.second);

	// Run the activity specific code
	retsize = a(args, &retbuf);

	// Postprocessing of the result and cleanup actions
	complete_activity(a, retbuf, retsize);
}


/* Utility function for Task::is_DAG(). Runs DFS starting from the specified node.
*  Returns true if it encounters a GREY node, false otherwise.
*/
bool Task::DFS_traverse(Activity& a) const
{
	// Mark GREY the activity node
	a.col = Color::GREY; 

	// For each successor
	for (auto &dep : a.dependent_ops) {
		Activity& suc = *(dep.first);

		// If it is GREY, cycle has been spotted
		if (suc.col == Color::GREY)
			return true;
		// If white, propagate the DFS
		if (suc.col == Color::WHITE && DFS_traverse(suc))
	 		return true;
	}
	// Mark this vertex as processed eventually
	a.col = Color::BLACK; 
	return false; 
}



/* Checks if the activities belonging to a task are arranged as a DAG (Directed Acyclic Graph).
*  Returns true if so, false otherwise.
*/
bool Task::is_DAG() const
{
	// Initialize colors to WHITE
	for (auto &a : activities)
		a->col = Color::WHITE;
  
	// For each vertex, DFS traverse
	for (auto &a : activities) {
		if (a->col == Color::WHITE)
			if (DFS_traverse(*a))
				return false;
	}
	return true;
}


/* Constructor */
Task::Task(void*& res) :
	id(ID_gen++),
	has_endpoint(false),
	completed(false),
	result(&res)
{}


/* Destructor */
Task::~Task() {}


/* Bind an activity to its task. */
void Task::add_activity(Activity& a)
{
	assert(!(a.is_endpoint && this->has_endpoint) && "Task can't have multiple endpoints!");
	assert(!a.owner && "Activity can't belong to multiple tasks!");

	a.owner = this;

	if (a.is_endpoint) {
		a.final_res = result;
		this->has_endpoint = true;
	}
	activities.emplace_back(&a);
}


/* Declare a dependency relationship between two activities. */
int Task::add_dependency(Activity& a_src, Activity& a_dst)
{
	// Make sure the activities belong to the task
	if (a_src.owner != this || a_dst.owner != this)
		return -1;
	// Make sure src and dst are different activities
	if (&a_src == &a_dst)
		return -2;
	// The src activity shall not be the final one
	if (a_src.is_endpoint)
		return -3;
	// Make sure the dependency (src->dst) doesn't already exist
	if (a_src.dependent_ops.find(&a_dst) != a_src.dependent_ops.end())
		return -4;

	// Notify the first activity node about the dependency (without specifying ret port for now)
	a_src.dependent_ops[&a_dst] = -1;
	// Notify the second activity node about the dependency (allocating space for a funneled arg)
	a_dst.params.emplace_back(std::make_pair(false, nullptr));
	a_dst.n_unresolved++;

	return 0;
}


/* Specifiy how the return value of an activity should piped into the 
*  parameters list of its dependant activities. 
*  a_src sends its result to a_dst, port is the index in the param list. */
int Task::link_ret_to_arg(Activity& a_src, Activity& a_dst, unsigned port)
{
	// Make sure the dependency (src->dst) was already declared
	if (a_src.dependent_ops.find(&a_dst) == a_src.dependent_ops.end())
		return -1;
	// Make sure the port is valid
	if (port == 0 || port >= a_dst.params.size())
		return -2;
	// Make sure the port is not already used
	if (a_dst.params[port].first == true)
		return -3;

	// Bind
	a_src.dependent_ops[&a_dst] = port;
	a_dst.params[port].first = true;

	return 0;
}


/* Ostream operator (for debug/monitoring purposes) */
std::ostream& operator<<(std::ostream& os, const Task& t) {

	os << "================" << std::endl;
	os << "Task: " << t.id << std::endl;
	for (auto const &a : t.activities) {
		os << *a << std::endl;
	}
	std::cout << "Is Directed Acyclic Graph: " << (t.is_DAG() ? "yes" : "no") << std::endl;
	os << "================" << std::endl;
	return os;
}
