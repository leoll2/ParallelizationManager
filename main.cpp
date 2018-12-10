#include <iostream>

#include "manager.hpp"


void *op1(void *arg)
{
	std::cout << "OP1 executed" << std::endl;

	return NULL;
}


void *op2(void *arg)
{
	std::cout << "OP2 executed" << std::endl;

	return NULL;
}



int main() {

	int a1, a2;

	Manager(8);
	Task t;
	a1 = t.add_activity(op1, NULL);
	a2 = t.add_activity(op2, NULL);
	t.link_ret_to_arg(a1, a2, 0);

	return 0;
}