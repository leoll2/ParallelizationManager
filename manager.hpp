#ifndef _MANAGER_HPP
#define _MANAGER_HPP

#include <iostream>


class Activity
{
	private:
		void *(*func)(void *);
		void *direct_arg;		// direct arguments (not coming from dependencies)
		void *funnel_arg;		// arguments generated as results of dependencies
	public:
		Activity();
		~Activity();
	
};



class Task
{
	public:
		Task();
		~Task();
		int add_activity(void *(*func)(void *), void *arg);
		int add_precedence(int a1, int a2);
		int link_ret_to_arg(int src, int dst, unsigned port);
};



class Manager
{
	private:

	public:
		const unsigned n_threads;

		Manager(unsigned pool_size);
		~Manager();
		void add_task(Task& t);
};

#endif