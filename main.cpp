#include <iostream>

#include "manager.hpp"


class A_Activity : public Activity {
	public: 
		A_Activity(void *direct_arg) :
			Activity(direct_arg)
		{}
		virtual unsigned operator() (const std::vector<void *>& arg) {
			std::cout << "I'm A_Activity" << std::endl;
			return 8;
		}
};


class B_Activity : public Activity {
	public: 
		B_Activity(void *direct_arg) :
			Activity(direct_arg)
		{}
		virtual unsigned operator() (const std::vector<void *>& arg) {
			std::cout << "I'm B_Activity" << std::endl;
			return 8;
		}
};


int main() {

	int ret;

	Manager m(8);

	Task t;

	A_Activity a1(NULL);
	A_Activity a2(NULL);
	B_Activity a3(NULL);
	B_Activity a4(NULL);
	A_Activity a5(NULL);

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