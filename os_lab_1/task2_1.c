/**
		\file		: 		task2_1.c
		\author		:		Sreeram Sadasivam
		\brief 		:		Understanding usage of pthreads.
*/

#include "common.h"

/** Macros*/

int my_value = 42;
/**
		\brief:	Function which implements task 2.1: Multithreading with POSIX Threads
*/
void *task_2_1_function() {

//	sleep for 150 milliseconds
	usleep(150000);
	my_value = 18951;
	fprintf(stderr, "%s%d\n", "Child Thread Execution. My Value ",my_value);

	//The child thread sleeps for 500ms.
	usleep(500000);
	pthread_exit(0);
}
	

int main() {

	pthread_t custom_thread;

	/** Thread creation */
	int err = pthread_create(&custom_thread,NULL,task_2_1_function,NULL);
		
	if(err!=0) {

		exit(EXIT_FAILURE);
	}

	// sleep for 150 ms
	usleep(150000);
	fprintf(stderr, "%s%d\n", "Parent Thread Execution. My Value ",my_value);
	err = pthread_join(custom_thread,NULL);
	if(err!=0) {

		exit(EXIT_FAILURE);
	}

	return 0;
}
