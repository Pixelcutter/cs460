Name: Jared Diamond
Group: None

Project 2: CPU Scheduler
From supplied assignment description: "In this assignment, you are asked to implement a program which will use a minimum of three
threads to allow us to measure the performance (i.e. throughput, turnaround time, and waiting time in the ready queue) of the four
basic CPU scheduling algorithms: First Come First Serve (FCFS), Shortest Job First (SJF), Priority (PR), and Round-Robin (RR).
Your program will be emulating/simulating the processes whose priority, sequence of CPU burst time and I/O burst time will be
given in an input file."

Burst times are simulated with a sleep call for the appropriate amount of milliseconds.

Zip Contents:

    - c_files:
    	- fcfs.c: 	first come first serve scheduling algorithm code
    	- rr.c:   	round robin scheduling algorithm code
    	- pr.c:   	priority scheduling algorithm code
    	- sjf.c:  	shortest job first scheduling algorithm code
    	- utilFuncs.c: 	utility functions that manipulate the queues, get current time, free memory, etc.
    	- printStats.c:	functions that calculate and print the final output
    	- main.c:	starting point of all threads and the main file
    	- parser.c:	parser thread code
    	- io.c: 	io thread code

    - h_files:
    	- global.h:	contains proc and queue data structures as well as all global variables defined in main.c
    	- *.h: 		files that have the same name as c_files contain function prototypes for their respective c files

    - Makefile: Standard makefile for easy compiling
    	- "make": 	compiles program and stuffs all .o files into the generated directory, 'build'.
    	- "make run": 	runs program without any arguments
    	- "make clean": deletes build directory and generated executable

How I share data between parts of the program:
Some data is passed between functions, but generally, especially if it's needed in multiple threads, it will be a global variable found in global.h.
Queues (ready, io, done), mutexes, arrival/finish times, parser/cpu status flags, and procs seen/completed variables are all global.

My approach to synchronization:
I use four mutexes to handle synchronization issues: a ready queue mutex/condition variable pair and an io queue mutex/condition variable pair.
The mutexes are used whenever their respective queues need to be manipulated, and the condition variables are used whenever their respective
queues are empty, but its likely that there will be something inserted into them.

How I switch between scheduling algorithms:
The scheduling algorithm used is determined by comparing the string found at argv[2] with either "FCFS", "SJF", "PR", or "RR". If argv[2] is not one
of the expected algorithms, the program will exit with a usage error message.

How I generate data for the required measurements:
The proccess data structure contains fields that keep track of burst time, wait time, arrival time, and finish time. Every time the cpu thread finishes
a process, it is added to a third queue, named doneQueue. When the cpu thread has been joined by the main thread, the done queue is looped through
and the final calculations are printed to the screen. The process data structure and final calculations are as follows:

    typedef struct process{
    		int priority;			// used by PR algorithm
    		int totalBurstTime;		// incremented whenever a cpu burst is found in schedule
    		int readyQueueWaitTime;		// incremented whenever dequeued from readyQueue
    		double arrivalTimeMillis;	// set when initialized
    		double finishTimeMillis;	// set when finished in cpu thread
    		double readyEnqueueTimeMillis;	// set whenever pushed into readyQueue

    		int nextIndex;			// next schedule index to be processed
    		int scheduleLen;		// length of schedule
    		int *schedule;			// parsed schedule

    		struct process *prevProc;
    		struct process *nextProc;
    } process;

    - avg turnaround time = sum([proc.finishTime - proc.arrivalTime for proc in doneQueue])/doneQueue.length
    - avg wait time = sum([proc.readyQueueWaitTime for proc in doneQueue])/doneQueue.length
    - throughput = doneQueue.length / ( program end time - program start time )
