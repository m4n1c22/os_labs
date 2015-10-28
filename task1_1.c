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

void task_1_1_function() {

	pid_t return_fork;
	pid_t return_wait;
	return_fork = fork();
	usleep(150000);
	if(return_fork<0) {
		
		fprintf(stderr, "%s\n","Error in execution of fork function.");
		exit(-1);
	
	}	else if(return_fork==0) {

		my_value = 18951;
		fprintf(stderr, "%s%d\n", "Child Process Execution. My Value ",my_value);
		usleep(500000);
	}	else {


		fprintf(stderr, "%s%d\n", "Parent Process Execution. My Value ",my_value);
		return_wait = wait(NULL);
		if(return_wait == -1) {

			fprintf(stderr, "%s\n","Error in execution of wait function.");
			exit(-1);		
		}
		fprintf(stderr, "%s%d\n", "Parent PID: ",getpid());
		fprintf(stderr, "%s%d\n", "Child PID: ",return_fork);
	}
}

int main() {

	task_1_1_function();
	return 0;
}
