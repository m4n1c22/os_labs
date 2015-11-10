/**
		\file		: 		task5_1.c
		\author		:		Sreeram Sadasivam
		\brief 		:		Understanding IPC with Shared memory in Linux.
*/
#include "common.h"

//Global variable generated for the processes.
int my_value = 42;

/**
		\brief:	Function which implements the task 5.1. Implements IPC with Shared Memory.
		\param scenario    Input which determines is being used. It can be scenario before wait
						   or after wait.
				
*/
void task_5_1_function(eSCENARIO scenario) {

	char *buffer;
	
	//set shared memory name
	const char *shm_name = "/DEEDS_lab1_shm";
	int shm_fd;

	pid_t return_fork;
	pid_t return_wait;

	//fork a child process
	return_fork = fork();

	//Check if there is error in forking a child process
	if(return_fork<0) {		
		fprintf(stderr, "%s\n","Error in execution of fork function.");
		exit(EXIT_FAILURE);
	
	}
	/*
		Condition block run by the parent process since, fork function returns pid
		of the child process as the  return value for function fork().
	*/
	else if(return_fork > 0){
		
		//sleep for 150 ms
		usleep(150000);
		
		/* 
			Shared memory Object(SMO) creation with the given shared memory name. 
			Shared file desciptor object is created as a handle for the SMO.
		*/
		shm_fd = shm_open ( shm_name , O_CREAT | O_RDWR, 0666);

		//Condition block to see if there was any error in the shared memory creation.    	   
		if (shm_fd == -1) {
			fprintf(stderr,"%s\n","Shared memory failed");
			exit(EXIT_FAILURE);
		}

		//set SMO length; returns 0 if OK, -1 on error
    	if(ftruncate (shm_fd, BUFFER_SIZE)==-1) {
			fprintf(stderr,"%s\n","Shared memory length alteration failed");
			exit(EXIT_FAILURE);    		
    	}

		//Performing a memory map and store in buffer
    	buffer = (char*) mmap ( NULL , BUFFER_SIZE, PROT_READ | PROT_WRITE,MAP_SHARED , shm_fd, 0);

    	//Condition block to see if there was a map failure or not.
    	if(buffer == MAP_FAILED) {
			fprintf(stderr, "%s\n", "Memory mapping failed...");    		
    	}

		fprintf(stdout, "%s%d\n", "Parent Process Execution. My Value ",my_value);
		
		sprintf(buffer,"Hi, I am your parent. My PID=%d and my_value=%d", getpid(), my_value);

		/* First Scenario, Parent reads the buffer content before the execution of wait call for child process.*/
		if(scenario == SCENARIO_BEFORE_WAIT) {
			
			//Incrementing the buffer pointer to read the new data in the buffer
			buffer += strlen(buffer);			
			fprintf(stdout,"Received message at parent is %s\n",buffer);						
			return_wait = wait(NULL);
			//Error handling for the wait() function call.
			if(return_wait == -1) {

				fprintf(stderr, "%s\n","Error in execution of wait function.");
				exit(EXIT_FAILURE);		
			}			
		}
		/* Second Scenario, Parent reads the buffer content after the execution of wait call for child.*/
		else if(scenario == SCENARIO_AFTER_WAIT) {
			
			//Incrementing the buffer pointer to read the new data in the buffer
			buffer += strlen(buffer);												
			return_wait = wait(NULL);
			//Error handling for the wait() function call.
			if(return_wait == -1) {

				fprintf(stderr, "%s\n","Error in execution of wait function.");
				exit(EXIT_FAILURE);		
			}
			fprintf(stdout,"Received message at parent is %s\n",buffer);
		}		
		fprintf(stdout, "%s%d\n", "Parent PID: ",getpid());
		fprintf(stdout, "%s%d\n", "Child PID: ",return_fork);

		//Unlinking the memory block. This step is equivalent to memory deallocation.
		if (shm_unlink(shm_name) == -1) {
			printf("Error removing %s\n",shm_name);
			exit(EXIT_FAILURE);
		}	

	}
	/*
		Condition block run by the child process since, fork function returns 0 
		as the return value for function fork().
	*/
	else {

		//Sleep for 150ms
		usleep(150000);
		my_value = 18951;
		//Opening the shared memory for reading and writing by child process.
		shm_fd = shm_open ( shm_name , O_RDWR, 0666);
		
		//Condition block to see if there was any error in the shared memory access.    	   
		if (shm_fd == -1) {
			fprintf(stderr,"%s\n","Shared memory failed");
			exit(EXIT_FAILURE);
		}
		fprintf(stdout, "%s%d\n", "Child Process Execution. My Value ",my_value);
    	//Sleep 500ms
    	usleep(500000);

		//Performing a memory map and store in buffer for the given SMO
    	buffer = (char*) mmap ( NULL , BUFFER_SIZE, PROT_READ | PROT_WRITE,MAP_SHARED , shm_fd, 0);

    	//Condition block to see if there was a map failure or not.
    	if(buffer == MAP_FAILED) {
			fprintf(stderr, "%s\n", "Memory mapping failed...");    		
    	}		
		fprintf(stdout,"Received message at child is %s\n",buffer);
		//Incrementing the buffer pointer to write the new data in the buffer
		buffer += strlen(buffer);
		sprintf(buffer,"Hi, I am your child. My PID=%d and my_value=%d", getpid(), my_value);
		exit(EXIT_SUCCESS);
	}
}

int main(int argc,char *argv[]) {

	fprintf(stdout,"%s\n","SCENARIO_BEFORE_WAIT");
	task_5_1_function(SCENARIO_BEFORE_WAIT);
	fprintf(stdout,"%s\n","SCENARIO_AFTER_WAIT");
	task_5_1_function(SCENARIO_AFTER_WAIT);
	
	return 0;
}
