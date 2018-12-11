#include "manager.hpp"


Task::Activity::Activity(void *(*func)(void *), void *arg) :
	routine(func),
	direct_args(arg),
	n_unresolved(0)
{}


Task::Activity::~Activity() {}


std::ostream& operator<<(std::ostream& os, const Task::Activity& a) {

	os << "Activity" << std::endl;
	os << "Funnel args: ";
	for (auto const &fa : a.funnel_args)
		os << (fa.first ? "1 " : "0 ");
	os << std::endl;
	os << "# of unresolved dependencies " << a.n_unresolved << std::endl;
	os << "Dependent ops: ";
	for (auto const &dep : a.dependent_ops)
		os << "(" << dep.first << ", " << dep.second << ")";
	os << std::endl;
	return os;
}


bool Task::DFS_traverse(int a, std::vector<Color>& colors)
{
	// Mark GREY the node with index a
	colors[a] = GREY; 
  
    // For each successor
    for (auto it = activities[a].dependent_ops.begin(); it != activities[a].dependent_ops.end(); ++it) {
    	int suc = it->first;

    	// If it is GREY, cycle has been spotted
    	if (colors[suc] == GREY)
    		return false;

    	// If white, propagate the DFS
    	if (colors[suc] == WHITE && DFS_traverse(suc, colors))
    		return false;
    }
    // Mark this vertex as processed eventually
    colors[a] = BLACK; 
  
    return true; 
}


bool Task::is_DAG()
{
	// Initialize colors to WHITE
	std::vector<Color> colors(activities.size(), WHITE);
  
    // For each vertex, DFS traverse
	for (auto &c : colors) {
		if (c == WHITE)
			if (DFS_traverse(&c - &colors[0], colors))
				return false;
	}
  
    return true;
}


Task::Task() {}


Task::~Task() {}


int Task::add_activity(void *(*func)(void *), void *arg)
{
	activities.emplace_back(Activity(func, arg));
	return activities.size() - 1;
}


int Task::add_dependency(int src, int dst)
{
	// Make sure the activities exist
	if (src < 0 || dst < 0 || src >= activities.size() || dst >= activities.size())
		return -1;
	// Make sure src and dst are different activities
	if (src == dst)
		return -2;
	// Make sure the dependency (src->dst) doesn't already exist
	if (activities[src].dependent_ops.find(dst) != activities[src].dependent_ops.end())
		return -3;

	// Notify the first activity node about the dependency (without specifying ret port for now)
	activities[src].dependent_ops[dst] = -1;
	// Notify the second activity node about the dependency (allocating space for a funneled arg)
	activities[dst].funnel_args.emplace_back(std::make_pair(false, nullptr));
	activities[dst].n_unresolved++;

	return 0;
}


int Task::link_ret_to_arg(int src, int dst, unsigned port)
{
	// Make sure the dependency (src->dst) was already declared
	if (activities[src].dependent_ops.find(dst) == activities[src].dependent_ops.end())
		return -1;

	// Make sure the port exists
	if (port >= activities[dst].funnel_args.size())
		return -2;
	// Make sure the port is not already used
	if (activities[dst].funnel_args[port].first == true)
		return -3;

	// Bind
	activities[src].dependent_ops[dst] = port;
	activities[dst].funnel_args[port].first = true;

	std::cout << "Bound return to argument" << std::endl;
	return 0;
}


std::ostream& operator<<(std::ostream& os, const Task& t) {

	os << "=== Task ===" << std::endl;
	for (auto const &a : t.activities) {
		os << a << std::endl;
	}
	os << "============" << std::endl;
	return os;
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