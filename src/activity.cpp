#include <iostream>

#include "activity.hpp"
#include "debug.hpp"


int Activity::ID_gen = 0;

Activity::Activity(void *direct_arg, bool endp) :
	id(ID_gen++),
	owner(nullptr),
	n_unresolved(0),
	is_endpoint(endp)
{
	params.emplace_back(std::make_pair(true, direct_arg));
}


Activity::~Activity()
{}


std::ostream& operator<<(std::ostream& os, const Activity& a)
{

	os << "Activity #" << a.id << std::endl;
	os << "No. of parameters: ";
	unsigned count = 0;
	for (auto const &arg : a.params)
		count += (arg.first) ? 1 : 0;
	os << count << std::endl;
	os << "No. of dependencies: " << a.n_unresolved << std::endl;
	os << "Dependent activities (id, [port]): " << std::endl;
	for (auto const &dep : a.dependent_ops)
		os << "(" << (*(dep.first)).id << " [" << dep.second << "])";
	os << std::endl;
	return os;
}
