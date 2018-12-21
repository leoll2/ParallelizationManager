/*#############################
#
# Test_2
#
# Activities:
# a = 8
# b = a + 5
# c = 2 * b
# d = b - 1
# e = c + d
#
# Returned variable:
# e
#
# Expected result:
# 38
#
#############################*/

#define EXPECTED_RES    38

#include <cassert>
#include <iostream>

#include "parallelizer.hpp" 




ACTIVITY(A_Activity)
{
	int res = 8;
	//std::this_thread::sleep_for (std::chrono::seconds(1));

	RETURN(res)
}


ACTIVITY(B_Activity)
{
	GET_ARG(a, int, 1)

	int res = a + 5;
	//std::this_thread::sleep_for (std::chrono::seconds(1));
	
	RETURN(res)
}

ACTIVITY(C_Activity)
{
	GET_ARG(a, int, 1)

	int res = 2 * a;
	//std::this_thread::sleep_for (std::chrono::seconds(1));

	RETURN(res)
}


ACTIVITY(D_Activity)
{
	GET_ARG(a, int, 1)

	int res = a - 1;
	//std::this_thread::sleep_for (std::chrono::seconds(1));

	RETURN(res)
}


ACTIVITY(E_Activity)
{
	GET_ARG(a, int, 1)
	GET_ARG(b, int, 2)

	int res = a + b;
	//std::this_thread::sleep_for (std::chrono::seconds(1));
	
	RETURN(res)
}


int main() {

	void *ret;
	int result;

	PManager m(8);

	Task t(ret);

	A_Activity a1(NULL);
	B_Activity a2(NULL);
	C_Activity a3(NULL);
	D_Activity a4(NULL);
	E_Activity a5(NULL, true);

	t.add_activity(a1);
	t.add_activity(a2);
	t.add_activity(a3);
	t.add_activity(a4);
	t.add_activity(a5);

	t.add_dependency(a1, a2);
	t.add_dependency(a2, a3);
	t.add_dependency(a2, a4);
	t.add_dependency(a3, a5);
	t.add_dependency(a4, a5);

	t.link_ret_to_arg(a1, a2, 1);
	t.link_ret_to_arg(a2, a3, 1);
	t.link_ret_to_arg(a2, a4, 1);
	t.link_ret_to_arg(a3, a5, 1);
	t.link_ret_to_arg(a4, a5, 2);


	D(std::cout << t << std::endl);

	m.add_task(t);
	m.run();

	RETRIEVE_RESULT(result, ret, int);

    std::cout << "Test 2" << std::endl;
    std::cout << "Expected result: " << EXPECTED_RES << std::endl;
	std::cout << "Final result: " << result << std::endl;

	assert((result == EXPECTED_RES) && "Wrong result!");

	return 0;
}