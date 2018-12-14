#include <iostream>

#include "manager.hpp"


ACTIVITY(A_Activity)
{
	std::cout << "I'm A_Activity" << std::endl;
	int a = 14;

	RETURN(a)
	//NO_RETURN()
}


ACTIVITY(B_Activity)
{
	std::cout << "I'm B_Activity" << std::endl;
	
	NO_RETURN()
}

ACTIVITY(C_Activity)
{
	std::cout << "I'm C_Activity" << std::endl;
	
	NO_RETURN()
}


int main() {

	int ret;

	Manager m(8);

	Task t;

	A_Activity a1(NULL);
	A_Activity a2(NULL);
	B_Activity a3(NULL);
	B_Activity a4(NULL);
	C_Activity a5(NULL);

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
	std::cout << "Checking if the task is Directed Acyclic Graph: " << (t.is_DAG() ? "yes" : "no") << std::endl;

	m.add_task(t);
	m.run_task();

	return 0;
}