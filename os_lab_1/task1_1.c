/**
		\file		: 		task1_1.c
		\author		:		Sreeram Sadasivam
		\brief 		:		Understanding fork() and wait() system calls.
*/

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>


//Global variable generated for the processes.
int my_value = 42;

/**
		\brief:	Function which implements the task 1.1.
				It discusses the implementation of fork and wait system
				calls.
*/
void task_1_1_function() {

	pid_t return_fork;
	pid_t return_wait;

	return_fork = fork();
	
	// Error handling for function fork().
	if(return_fork<0) {
		
		fprintf(stderr, "%s\n","Error in execution of fork function.");
		exit(EXIT_FAILURE);
	
	}	
	/*
		Condition block run by the child process since, fork function returns 0 
		as the return value for function fork().
	*/
	else if(return_fork==0) {

		//The process sleeps for 150ms.
		usleep(150000);
		
		//my_value is modified by the child process from 42 to a different value.
		my_value = 18951;
		fprintf(stderr, "%s%d\n", "Child Process Execution. My Value ",my_value);
		fprintf(stderr, "%s%d\n", "From Child -> PID: ",getpid());

		//The child process sleeps for 500ms.
		usleep(500000);
	}
	/*
		Condition block run by the parent process since, fork function returns pid
		of the child process as the  return value for function fork().
	*/
	else {
		
		//The process sleeps for 150ms.
		usleep(150000);
		fprintf(stderr, "%s%d\n", "Parent Process Execution. My Value ",my_value);
		return_wait = wait(NULL);

		//Error handling for the wait() function call.
		if(return_wait == -1) {

			fprintf(stderr, "%s\n","Error in execution of wait function.");
			exit(EXIT_FAILURE);		
		}

		fprintf(stderr, "%s%d\n", "Parent PID: ",getpid());
		fprintf(stderr, "%s%d\n", "Child PID: ",return_fork);
	}
}

int main() {

	task_1_1_function();
	return 0;
}
