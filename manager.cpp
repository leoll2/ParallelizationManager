#include <cassert>

#include "manager.hpp"

int Activity::ID_gen = 0;

Activity::Activity(void *direct_arg) :
	id(ID_gen++),
	n_unresolved(0)
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
		os << "(" << (*(dep.first)).id << ", " << dep.second << ")";
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


void Task::complete_activity(Activity& a, void *retvalue) {

	for (auto const &dep : a.dependent_ops) {
		Activity& a_next = *(dep.first);
		int port = dep.second;

		// Funnel the return value into the parameter list of the successor (if configured)
		if (port >= 0) {
			assert(a_next.params[port].first && "Attempt to write in a non-allocated port");
			a_next.params[port].second = retvalue;
		}
		// Resolve dependency for successor activities. If 0 left, put the activity in the ready queue.
		if (--a_next.n_unresolved == 0)
			ready_q.emplace(std::make_pair(&a_next, a_next.dependent_ops.size()));
	}
}

void Task::run_activity(Activity& a) {

	std::cout << "Executing activity #" << a.id << std::endl;
	std::vector<void *> args;
	args.reserve(a.params.size());
	for (auto const &arg : a.params)
		args.push_back(arg.second);

	a(args);			// NUOVO CODICE

	void *ret = NULL;	// TODO: DA IMPLEMENTARE
	complete_activity(a, ret);
}


bool Task::DFS_traverse(Activity& a)
{
	// Mark GREY the activity node
	a.col = Color::GREY; 

	// For each successor
	for (auto &dep : a.dependent_ops) {
		Activity& suc = *(dep.first);

		// If it is GREY, cycle has been spotted
		if (suc.col == Color::GREY)
			return false;

		// If white, propagate the DFS
    	if (suc.col == Color::WHITE && DFS_traverse(suc))
    		return false;
	}

    // Mark this vertex as processed eventually
    a.col = Color::BLACK; 
  
    return true; 
}


bool Task::is_DAG()
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


Task::Task() {}


Task::~Task() {}


void Task::add_activity(Activity& a)
{
	activities.emplace_back(&a);
}


int Task::add_dependency(Activity& a_src, Activity& a_dst)
{
	// Make sure the activities belong to the task
	//	return -1;

	// Make sure src and dst are different activities
	if (&a_src == &a_dst)
		return -2;

	// TODO: Make sure the dependency (src->dst) doesn't already exist
	if (a_src.dependent_ops.find(&a_dst) != a_src.dependent_ops.end())
		return -3;

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

	std::cout << "Bound return to argument" << std::endl;
	return 0;
}


std::ostream& operator<<(std::ostream& os, const Task& t) {

	os << "=== Task ===" << std::endl;
	for (auto const &a : t.activities) {
		os << *a << std::endl;
	}
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