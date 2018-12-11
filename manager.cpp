#include "manager.hpp"


Task::Activity::Activity(void *(*func)(void *), void *arg) :
	routine(func),
	direct_args(arg),
	n_unresolved(0)
{}


Task::Activity::~Activity() {}


Task::Task() {}


Task::~Task() {}


int Task::add_activity(void *(*func)(void *), void *arg)
{
	activities.emplace_back(Activity(func, arg));
	return 0;
}


std::set<std::pair<int, int> >::iterator find_dep(const std::set<std::pair<int, int> >& s, int id) {

	return std::find_if(s.begin(), s.end(), [&id] (const std::pair<int, int>& elem) { return elem.first == id; });
}


int Task::add_dependency(int src, int dst)
{
	// Make sure the activities exist
	if (src < 0 || dst < 0 || src >= activities.size() || dst >= activities.size())
		return -1;
	// Make sure src and dst are different activities
	if (src == src)
		return -2;
	// Make sure the dependency (src->dst) doesn't already exist
	/*if (std::find_if(activities[src].dependent_ops.begin(), activities[src].dependent_ops.end(), 
			[&dst] (const std::pair<int, int>& dep) { return dep.first == dst; }) != 
			activities[src].dependent_ops.end()
	)*/
	if (find_dep(activities[src].dependent_ops, dst) != activities[src].dependent_ops.end())
		return -3;

	// Notify the first activity node about the dependency (without specifying ret port for now)
	activities[src].dependent_ops.insert(std::make_pair(dst, -1));
	// Notify the second activity node about the dependency (allocating space for a funneled arg)
	activities[dst].funnel_args.emplace_back(std::make_pair(false, nullptr));
	activities[dst].n_unresolved++;

	return 0;
}


int Task::link_ret_to_arg(int src, int dst, unsigned port)
{
	// Make sure the dependency (src->dst) was already declared
	if (find_dep(activities[src].dependent_ops, dst) == activities[src].dependent_ops.end())
		return -1;

	// Make sure the port exists
	if (port >= activities[dst].funnel_args.size())
		return -2;
	// Make sure the port is not already used
	if (activities[dst].funnel_args[port].first == true)
		return -3;

	// Bind
	//(*find_dep(activities[src].dependent_ops, dst)).second = port;
	activities[dst].funnel_args[port].first = true;

	std::cout << "Bound return to argument" << std::endl;
	return 0;
}


Manager::Manager(unsigned pool_size) : n_threads(pool_size)
{

	/* TODO: allocate a pool of threads*/
}


Manager::~Manager()
{

	/* TODO: kill threads of the pool */
}


void Manager::add_task(Task& t) {

	/* TODO: aggiungi task */
}