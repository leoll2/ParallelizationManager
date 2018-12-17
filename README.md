# ParallelizationManager
ParallelizationManager helps you writing parallelizable programs and execute their operations on multiple threads concurrently. In other words, ParallelizationManager is a thread pool scheduler for parallelizable tasks whose dependencies can be modeled as a Directed Acyclic Graph (DAG).

## Obtain

Download with:

```
git clone https://www.github.com/leoll2/ParallelizationManager.git
```

Compile with:

```
make
```


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
where `<Parameter>` is the name we use to refer to the argument, `<Type>` is the data type and `<Port>` is the index of this argument among all those passed to the activity (must be the same specified in `link_ret_to_arg()`.

**Example**
For instance, this is what the activity of adding two integers looks like:

```
ACTIVITY(Sum)
{
	GET_ARG(a, int, 1)
	GET_ARG(b, int, 2)

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
