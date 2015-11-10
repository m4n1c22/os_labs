#include "common.h"

//Global variable generated for the processes.
int my_value = 42;

/**
		\brief:	Function which implements the task 5.1. Implements IPC with Shared Memory
				
*/
void task_5_1_function(eSCENARIO scenario) {

	char *buffer;
//	set shared memory name
	const char *shm_name = "/DEEDS_lab1_shm";
	int shm_fd;

	pid_t return_fork;
	pid_t return_wait;

//	fork a child process
	return_fork = fork();

//	Check if there is error in forking a child process
	if(return_fork<0) {		
		fprintf(stderr, "%s\n","Error in execution of fork function.");
		exit(EXIT_FAILURE);
	
	}
//		if child forking of child process successful
	else if(return_fork > 0){
//		sleep for 150 ms
		usleep(150000);
//		creates and opens SMO; returns SMO handle, -1 on error
		shm_fd = shm_open ( shm_name , O_CREAT | O_RDWR, 0666);    	   
		if (shm_fd == -1) {
			fprintf(stderr,"%s\n","Shared memory failed");
			exit(EXIT_FAILURE);
		}

//		set SMO length; returns 0 if OK, -1 on error
    	ftruncate (shm_fd, BUFFER_SIZE);

//		create a memory map and store in buffer
    	buffer = (char*) mmap ( NULL , BUFFER_SIZE, PROT_READ | PROT_WRITE,MAP_SHARED , shm_fd, 0);

    	if(buffer == MAP_FAILED) {
			fprintf(stderr, "%s\n", "Memory mapping failed...");    		
    	}

		fprintf(stdout, "%s%d\n", "Parent Process Execution. My Value ",my_value);
		
		sprintf(buffer,"Hi, I am your parent. My PID=%d and my_value=%d", getpid(), my_value);


		if(scenario == SCENARIO_BEFORE_WAIT) {
			
			buffer += strlen(buffer);			
			fprintf(stdout,"Received message at parent is %s\n",buffer);						
			return_wait = wait(NULL);

			if(return_wait == -1) {

				fprintf(stderr, "%s\n","Error in execution of wait function.");
				exit(EXIT_FAILURE);		
			}			
		}
		else if(scenario == SCENARIO_AFTER_WAIT) {
			
			buffer += strlen(buffer);												
			return_wait = wait(NULL);

			if(return_wait == -1) {

				fprintf(stderr, "%s\n","Error in execution of wait function.");
				exit(EXIT_FAILURE);		
			}
			fprintf(stdout,"Received message at parent is %s\n",buffer);
		}		


		fprintf(stdout, "%s%d\n", "Parent PID: ",getpid());
		fprintf(stdout, "%s%d\n", "Child PID: ",return_fork);
		
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

		
		usleep(150000);
		my_value = 18951;

		shm_fd = shm_open ( shm_name , O_RDWR, 0666);

		if (shm_fd == -1) {
			fprintf(stderr,"%s\n","Shared memory failed");
			exit(EXIT_FAILURE);
		}
		fprintf(stdout, "%s%d\n", "Child Process Execution. My Value ",my_value);
    	
    	usleep(500000);

    	buffer = (char*) mmap ( NULL , BUFFER_SIZE, PROT_READ | PROT_WRITE,MAP_SHARED , shm_fd, 0);

    	if(buffer == MAP_FAILED) {
			fprintf(stderr, "%s\n", "Memory mapping failed...");    		
    	}		
		fprintf(stdout,"Received message at child is %s\n",buffer);
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
