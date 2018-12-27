# ParallelizationManager
ParallelizationManager helps you writing parallelizable programs and execute their operations on multiple threads concurrently. In other words, ParallelizationManager is a thread pool scheduler for parallelizable tasks whose dependencies can be modeled as a Directed Acyclic Graph (DAG).

## Table of Contents
- [Obtain](#obtain)
- [Usage](#usage)
  - [Activity](#activity)

## Obtain

Download with:

```
git clone https://www.github.com/leoll2/ParallelizationManager.git
```

Compile with:

```
make
```

## Nomenclature
These terms will be extensively used in the following description. Here is a clarification about their meaning in this context.
- Activity: a self-contained sequence of instructions (that is, C++ code) which is to be executed by one thread.
- Task: a collection of activities, arranged as a DAG by precedence; independent activities can be executed in parallel.
- Manager: entity which decides the next activity to execute, according to a scheduling policy, and assigns it to an available worker.
- Worker: basically a thread which can execute one activity at a time.

## Usage

### Activity

An activity is declared according to the following syntax, where `<Name>` is the indeed the name of the activity, and `<Code>` contains the actual instructions to be executed.

```
ACTIVITY(<Name>)
{
	<Code>
}
```

**Arguments**

If an activity needs arguments to execute, these can be retrieved with:
```
GET_ARG(<Parameter>, <Type>, <Port>)
```
where `<Parameter>` is the local variable to store the argument, `<Type>` is the data type and `<Port>` is the index of this argument among all those passed to the activity (must be the same specified in `link_ret_to_arg()`.

There is also a shortcut to declare the local variable and retrieve the argument at once:
```
DECL_AND_GET_ARG(<Parameter>, <Type>, <Port>)
```

**Example**
For instance, this is what the activity of adding two integers looks like:

```
ACTIVITY(Sum)
{
	DECL_AND_GET_ARG(a, int, 1)
	DECL_AND_GET_ARG(b, int, 2)

	int res = a + b;
	
	RETURN(res)
}
```

### Task

```
Task <Name>(<RetValPointer>);
```
where `<Name>` is the task name, and `<RetvalPointer>` shall be a void pointer to the location where the final result will be allocated.


### Manager
A thread pool manager is instantiated as follows:

```
PManager m(<PoolSize>);
```

where `<PoolSize>` is the number of allocated worker threads.
