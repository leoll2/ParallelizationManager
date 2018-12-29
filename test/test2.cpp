/*#############################
#
# Test_2
#
# Purpose: show a highly parallelizable task, whose number
# of ready activities sometimes exceeds the available workers
#
# Description: Compute the sum of the first 16 square numbers
#
# Parameters:
# i = 1..16
# x[i] = i
#
# Activities:
# a[i] = x[i]^2
# a_res = sum_i(a[i])
#
# Returned:
# a_res
#
# Expected result:
# 1240
#
#############################*/

#define EXPECTED_RES	1496

#include <algorithm>
#include <cassert>
#include <iostream>
#include <numeric>
#include <vector>

#include "parallelizer.hpp" 

const unsigned N = 16;


ACTIVITY(SquareOp)
{
	DECL_AND_GET_ARG(x, int, 0)

	int res = x * x;
	std::this_thread::sleep_for (std::chrono::milliseconds(100));

	RETURN(res)
}


ACTIVITY(SumOp)
{
	std::vector<int> squares(N);
	for (unsigned i = 1; i <= N; ++i)
		GET_ARG(squares[i-1], int, i)

	int res = std::accumulate(squares.begin(), squares.end(), 0);
	//std::this_thread::sleep_for (std::chrono::seconds(1));
	
	RETURN(res)
}



int main() {

	void *ret;
	int result;

	std::vector<int> values(N);
	std::iota(values.begin(), values.end(), 1);

	PManager m(8);

	Task t(ret);

	SumOp sum_op(NULL, true);
	t.add_activity(sum_op);

	std::vector<SquareOp*> sq_ops(N);
	for (unsigned i = 0; i < N; ++i) {
		sq_ops[i] = new SquareOp(&values[i]);
		t.add_activity(*sq_ops[i]);
		t.add_dependency(*sq_ops[i], sum_op);
		t.link_ret_to_arg(*sq_ops[i], sum_op, i+1);
	}

	D(std::cout << t << std::endl);

	m.add_task(t);

	m.run();

	RETRIEVE_RESULT(result, ret, int);

    for (auto &op : sq_ops) delete op;

	std::cout << "Test 2" << std::endl;
	std::cout << "Expected result: " << EXPECTED_RES << std::endl;
	std::cout << "Final result: " << result << std::endl;

	assert((result == EXPECTED_RES) && "Wrong result!");

	return 0;
}
