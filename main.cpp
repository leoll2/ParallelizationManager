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

void *op4(void *arg)
{
	std::cout << "OP4 executed" << std::endl;

	return NULL;
}

void *op5(void *arg)
{
	std::cout << "OP5 executed" << std::endl;

	return NULL;
}


int main() {

	int a1, a2, a3, a4, a5;
	int ret;

	Manager m(8);

	Task t;

	a1 = t.add_activity(op1, NULL);
	std::cout << "add activity returned with code " << a1 << std::endl;
	a2 = t.add_activity(op2, NULL);
	std::cout << "add activity returned with code " << a2 << std::endl;
	a3 = t.add_activity(op3, NULL);
	std::cout << "add activity returned with code " << a3 << std::endl;
	a4 = t.add_activity(op4, NULL);
	std::cout << "add activity returned with code " << a4 << std::endl;
	a5 = t.add_activity(op5, NULL);
	std::cout << "add activity returned with code " << a5 << std::endl;

	ret = t.add_dependency(a5, a2);
	//std::cout << "add dependency x->y returned with code " << ret << std::endl;
	ret = t.add_dependency(a2, a3);
	ret = t.add_dependency(a2, a4);
	ret = t.add_dependency(a3, a1);
	ret = t.add_dependency(a4, a1);

	//t.link_ret_to_arg(a1, a2, 0);


	std::cout << t << std::endl;
	std::cout << (t.is_DAG() ? "DAG" : "not DAG") << std::endl;

	m.add_task(t);
	m.run_task();

	return 0;
}