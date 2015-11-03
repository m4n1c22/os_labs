/**
		\file 			:		task5_1.c
		\author			:		Sreeram Sadasivam
		\brief			: 		Understanding the implementation of shared memory in operating system.
*/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>		
#include <sys/types.h>
#include <unistd.h>	

//Macros
#define BUFFER_SIZE 1024


//Global variable generated for the processes.
int my_value = 42;

/**
		\brief:	Function which implements the task 5.1. Implements IPC with Shared Memory
				
*/
void task_5_1_function() {

	char *buffer;
	const char *shm_name = "/DEEDS_lab1_shm";
	int shm_fd;

	pid_t return_fork;
	pid_t return_wait;

	
	return_fork = fork();

	if(return_fork<0) {		
		fprintf(stderr, "%s\n","Error in execution of fork function.");
		exit(EXIT_FAILURE);
	
	}	
	else if(return_fork > 0){
		
		usleep(150000);

		shm_fd = shm_open ( shm_name , O_CREAT | O_RDWR, 0666);    	   
		if (shm_fd == -1) {
			fprintf(stderr,"%s\n","Shared memory failed");
			exit(EXIT_FAILURE);
		}

    	ftruncate (shm_fd, BUFFER_SIZE);

    	buffer = (char*) mmap ( NULL , BUFFER_SIZE, PROT_READ | PROT_WRITE,MAP_SHARED , shm_fd, 0);

    	if(buffer == MAP_FAILED) {
			fprintf(stderr, "%s\n", "Memory mapping failed...");    		
    	}

		fprintf(stdout, "%s%d\n", "Parent Process Execution. My Value ",my_value);
		
		sprintf(buffer,"Hi, I am your parent. My PID=%d and my_value=%d", getpid(), my_value);

		return_wait = wait(NULL);

		if(return_wait == -1) {

			fprintf(stderr, "%s\n","Error in execution of wait function.");
			exit(EXIT_FAILURE);		
		}

		fprintf(stdout, "%s%d\n", "Parent PID: ",getpid());
		fprintf(stdout, "%s%d\n", "Child PID: ",return_fork);
		
		if (shm_unlink(shm_name) == -1) {
			printf("Error removing %s\n",shm_name);
			exit(EXIT_FAILURE);
		}	

		exit(EXIT_SUCCESS);
	}
	/*
		Condition block run by the child process since, fork function returns 0 
		as the return value for function fork().
	*/
	else {

		
		usleep(150000);
		my_value = 18951;

		shm_fd = shm_open ( shm_name , O_RDONLY, 0666);

		if (shm_fd == -1) {
			fprintf(stderr,"%s\n","Shared memory failed");
			exit(EXIT_FAILURE);
		}
		fprintf(stdout, "%s%d\n", "Child Process Execution. My Value ",my_value);
    	
    	usleep(500000);

    	buffer = (char*) mmap ( NULL , BUFFER_SIZE, PROT_READ,MAP_SHARED , shm_fd, 0);

    	if(buffer == MAP_FAILED) {
			fprintf(stderr, "%s\n", "Memory mapping failed...");    		
    	}		
		fprintf(stdout,"Received message at child is %s\n",buffer);
		exit(EXIT_SUCCESS);
	}
}

int main() {

	task_5_1_function();
	return 0;
}
