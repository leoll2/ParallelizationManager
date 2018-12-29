/*#############################
#
# Test_3
#
# Purpose: show how multiple tasks can be executed in parallel.
#
# Description:
# First task does ROR 32 times in a row on an unsigned int.
# Second task, does the same but with ROL.
# At the end, both task are expected to return the same value.
#
# Returned:
# (res1 - res2)
#
# Expected result:
# 0
#
#############################*/

#define EXPECTED_RES	0

#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>

#include "parallelizer.hpp" 

uint32_t foo = 0xDEADBEEF;


/* Return the value passed as argument */
ACTIVITY(CONST_GEN)
{
    DECL_AND_GET_ARG(x, uint32_t, 0)
    
    RETURN(x);
}


/* ROR one bit */
ACTIVITY(ROR)
{
	DECL_AND_GET_ARG(x, uint32_t, 1)

	uint32_t res = (x >> 1) + ((x & 1) << 31);
	//std::this_thread::sleep_for (std::chrono::milliseconds(20));

	RETURN(res)
}


/* ROL one bit */
ACTIVITY(ROL)
{
	DECL_AND_GET_ARG(x, uint32_t, 1)

	uint32_t res = (x << 1) + ((x & 0x80000000) >> 31);
	//std::this_thread::sleep_for (std::chrono::milliseconds(10));

	RETURN(res)
}


int main() {

	void *ret1, *ret2;
	uint32_t res1, res2;

	PManager m(2);

	Task t1(ret1);
	Task t2(ret2);

    CONST_GEN cg1(&foo);
    CONST_GEN cg2(&foo);
    t1.add_activity(cg1);
	t2.add_activity(cg2);

    // Build the two tasks as chains of 32 ROR or ROL, respectively
	std::vector<ROR*> ror_ops(32);
	std::vector<ROL*> rol_ops(32);
	for (unsigned i = 0; i < 32; ++i) {
		ror_ops[i] = new ROR(NULL, (i == 31));
		rol_ops[i] = new ROL(NULL, (i == 31));
		
		t1.add_activity(*ror_ops[i]);
		t2.add_activity(*rol_ops[i]);
		
		if (i > 0) {
			t1.add_dependency(*ror_ops[i-1], *ror_ops[i]);
			t1.link_ret_to_arg(*ror_ops[i-1], *ror_ops[i], 1);
			t2.add_dependency(*rol_ops[i-1], *rol_ops[i]);
			t2.link_ret_to_arg(*rol_ops[i-1], *rol_ops[i] ,1);
		} else {
		    t1.add_dependency(cg1, *ror_ops[i]);
			t1.link_ret_to_arg(cg1, *ror_ops[i], 1);
			t2.add_dependency(cg2, *rol_ops[i]);
			t2.link_ret_to_arg(cg2, *rol_ops[i], 1);
		}
	}

	D(std::cout << "Task ROR" << std::endl << t1 << std::endl);
	D(std::cout << "Task ROL" << std::endl << t2 << std::endl);

	// t1 is added before t2, so it has higher priority (FCFS)
	m.add_task(t1);
	m.add_task(t2);

	m.run();

	RETRIEVE_RESULT(res1, ret1, uint32_t);
	RETRIEVE_RESULT(res2, ret2, uint32_t);

	for (auto &op : ror_ops) delete op;
	for (auto &op : rol_ops) delete op;

	std::cout << "Test 3" << std::endl;
	std::cout << "Expected result: " << EXPECTED_RES << std::endl;
	std::cout << "Final result: " << res1 - res2 << std::endl;

	assert(((res1 - res2) == EXPECTED_RES) && "Wrong result!");

	return 0;
}
