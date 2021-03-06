#ifndef _ACTIVITY_HPP
#define _ACTIVITY_HPP

#include <atomic>
#include <cstdlib>
#include <cstring>
#include <map>
#include <memory>
#include <utility>
#include <vector>

#include "task.hpp"



//////////////////////////////////////////////////////////////////////////
//																		//
//  Handy MACROS to handle activities, their arguments and return value //
//																		//
//////////////////////////////////////////////////////////////////////////

#define NO_RETURN() \
	*_retbuf = nullptr; \
	return 0;


#define RETURN(arg) \
	unsigned _retsize = sizeof arg; \
	*_retbuf = malloc(_retsize); \
	std::memcpy(*_retbuf, (void*)&arg, _retsize); \
	return _retsize;


#define ACTIVITY(name) \
    class name : public Activity { \
		public: \
			name(void *direct_arg, bool endpoint = false) : Activity(direct_arg, endpoint) {} \
			virtual unsigned operator() (const std::vector<void*>& args, void **_retbuf); \
    }; \
    unsigned name::operator() (const std::vector<void*>& args, void **_retbuf)


#define DECL_AND_GET_ARG(name, type, port) \
    type name = *(type*)args[(port)];
    
#define GET_ARG(name, type, port) \
    name = *(type*)args[(port)];


#define RETRIEVE_RESULT(dst, src, type) \
    dst = *((type*)(src)); \
    free(src);



/* Node 'color', used in DAG validation algorithm:
*	WHITE: node not explored yet 
*	GREY: node currently being explored
*	BLACK: node explored and all its descendants */
enum class Color {WHITE, GREY, BLACK};


class Task;


//////////////////////////////////////////////////////////////////////////
//																		//
//  Class ACTIVITY describes an operation that is part of a larger task //
//																		//
//////////////////////////////////////////////////////////////////////////

class Activity
{
	private:
		static int ID_gen;      // ID generator
		int id;                 // ID
		Task *owner;            // task owning this activity
		std::vector<std::pair<bool, void*> > params; // routine arguments (port allocated, pointer to arg).
		std::map<Activity*, int> dependent_ops;      // activities directly depending on this (operation, ret value port)
		std::atomic_int n_unresolved;   // no. of yet unresolved dependencies (updated during execution)
		bool is_endpoint;       // is it the final task?
		void **final_res;       // if final task, pointer to buffer to store the result
		Color col;              // used for DAG verification

	public:	
		Activity(void *direct_arg, bool endpoint = false);
		virtual ~Activity();
		virtual unsigned operator() (const std::vector<void *>& args, void **_retbuf) = 0;
		friend std::ostream& operator<<(std::ostream& os, const Activity& a);
		friend class Task;
		friend class PManager;
};

#endif
