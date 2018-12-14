#include <cassert>

#include "manager.hpp"

int Activity::ID_gen = 0;

Activity::Activity(void *direct_arg, bool endp) :
	id(ID_gen++),
	n_unresolved(0),
	is_endpoint(endp)
{
	params.emplace_back(std::make_pair(true, direct_arg));
}


Activity::~Activity() {}


std::ostream& operator<<(std::ostream& os, const Activity& a) {

	os << "Activity #" << a.id << std::endl;
	os << "Parameters: ";
	for (auto const &arg : a.params)
		os << (arg.first ? "1 " : "0 ");
	os << std::endl;
	os << "# of unresolved dependencies " << a.n_unresolved << std::endl;
	os << "Dependent ops: ";
	for (auto const &dep : a.dependent_ops)
		os << "(" << (*(dep.first)).id << " [" << dep.second << "])";
	os << std::endl;
	return os;
}


void Task::init_ready_q()
{
	for (auto const &a : activities) {
		if (a->n_unresolved == 0)
			ready_q.emplace(std::make_pair(a, a->dependent_ops.size()));
	}

	assert(!ready_q.empty() && "No task schedulable!");
}


Activity* Task::schedule()
{
	if (ready_q.empty())
		return nullptr;
	auto ret = ready_q.top();
	ready_q.pop();
	return ret.first;
}


void Task::complete_activity(Activity& a, void *retvalue, unsigned retsize) {

	if (a.is_endpoint) {
		// Copy the result in the provided buffer
		*(a.final_res) = malloc(retsize);
		std::memcpy(*(a.final_res), retvalue, retsize);
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
			// Resolve dependency for successor activities. If 0 left, put the activity in the ready queue.
			if (--a_next.n_unresolved == 0)
				ready_q.emplace(std::make_pair(&a_next, a_next.dependent_ops.size()));
		}
	}

	// Deallocate actual arguments
	for (auto &arg : a.params) {
		if (arg.first && arg.second)
			free(arg.second);
	}

	// Deallocate result buffer
	if (retsize)
		free(retvalue);
}



/* Execute the specified activity */
void Task::run_activity(Activity& a) {

	void *retbuf;		// buffer to store the result
	unsigned retsize;	// length of the buffer

	D(std::cout << "Executing activity #" << a.id << std::endl;)

	// Set up the actual arguments
	std::vector<void *> args;
	args.reserve(a.params.size());
	for (auto const &arg : a.params)
		args.push_back(arg.second);

	// Run the activity specific code
	retsize = a(args, &retbuf);

	D(std::cout << "Activity #" << a.id << " returned " << retsize << " bytes " << std::endl;)

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


Task::Task(void*& res) :
	has_endpoint(false),
	result(&res)
{}


Task::~Task()
{}


void Task::add_activity(Activity& a)
{
	assert(!(a.is_endpoint && this->has_endpoint) && "Task can't have multiple endpoints!");

	if (a.is_endpoint) {
		a.final_res = result;
		this->has_endpoint = true;
	}

	activities.emplace_back(&a);
}


int Task::add_dependency(Activity& a_src, Activity& a_dst)
{
	// TODO: Make sure the activities belong to the task
	//	return -1;

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


std::ostream& operator<<(std::ostream& os, const Task& t) {

	os << "=== Task ===" << std::endl;
	for (auto const &a : t.activities) {
		os << *a << std::endl;
	}
	std::cout << "Is Directed Acyclic Graph: " << (t.is_DAG() ? "yes" : "no") << std::endl;
	os << "============" << std::endl;
	return os;
}



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