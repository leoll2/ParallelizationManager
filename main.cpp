#include <iostream>

#include "manager.hpp"

// TODO: template e classe per activities

unsigned op0(const std::vector<void *>& arg)
{
	std::cout << "OP0 executed" << std::endl;
	return 8;
}

unsigned op1(const std::vector<void *>& arg)
{
	std::cout << "OP1 executed" << std::endl;
	return 8;
}

unsigned op2(const std::vector<void *>& arg)
{
	std::cout << "OP2 executed" << std::endl;
	return 8;
}

unsigned op3(const std::vector<void *>& arg)
{
	std::cout << "OP3 executed" << std::endl;
	return 8;
}

unsigned op4(const std::vector<void *>& arg)
{
	std::cout << "OP4 executed" << std::endl;
	return 8;
}

int main() {

	int ret;

	Manager m(8);

	Task t;

	Activity a1(op0, NULL);
	Activity a2(op1, NULL);
	Activity a3(op2, NULL);
	Activity a4(op3, NULL);
	Activity a5(op4, NULL);

	t.add_activity(a1);
	t.add_activity(a2);
	t.add_activity(a3);
	t.add_activity(a4);
	t.add_activity(a5);

	ret = t.add_dependency(a1, a2);
	ret = t.add_dependency(a2, a3);
	ret = t.add_dependency(a2, a4);
	ret = t.add_dependency(a3, a5);
	ret = t.add_dependency(a4, a5);

	//t.link_ret_to_arg(a1, a2, 1);


	std::cout << t << std::endl;
	std::cout << (t.is_DAG() ? "DAG" : "not DAG") << std::endl;

	m.add_task(t);
	m.run_task();

	return 0;
}