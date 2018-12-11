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


void *op3(void *arg)
{
	std::cout << "OP3 executed" << std::endl;

	return NULL;
}


int main() {

	int a1, a2, a3;
	int ret;

	Manager(8);

	Task t;

	a1 = t.add_activity(op1, NULL);
	std::cout << "add activity returned with code " << a1 << std::endl;
	a2 = t.add_activity(op2, NULL);
	std::cout << "add activity returned with code " << a2 << std::endl;
	a3 = t.add_activity(op3, NULL);
	std::cout << "add activity returned with code " << a3 << std::endl;

	ret = t.add_dependency(a1, a2);
	std::cout << "add dependency a1->a2 returned with code " << ret << std::endl;
	ret = t.add_dependency(a2, a3);
	std::cout << "add dependency a2->a3 returned with code " << ret << std::endl;
	ret = t.add_dependency(a1, a3);
	std::cout << "add dependency a1->a3 returned with code " << ret << std::endl;

	t.link_ret_to_arg(a1, a2, 0);


	std::cout << t << std::endl;
	std::cout << (t.is_DAG() ? "DAG" : "not DAG") << std::endl;

	return 0;
}