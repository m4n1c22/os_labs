
#include "common.h"

//Global variable generated for the processes.
int my_value = 42;

/**
		\brief:	Function which implements the task 3.1. Implements IPC with Pipes
				
*/
void task_3_1_function() {

    int fd[2];
	char buffer[BUFFER_SIZE];
	
	pid_t return_fork;
	pid_t return_wait;
        
	

	/*open a pipe and check for errors in pipe creation*/

	if(pipe(fd)==-1){
	fprintf(stderr,"Pipe creation failed\n");	
	}
	else{
	fprintf(stderr,"pipe creation successful\n");
	}

	/*fork a child process*/
	return_fork = fork();
	
	/*check for errors in child process creation*/
	if(return_fork<0) {
		
		fprintf(stderr, "%s\n","Error in execution of fork function.");
		exit(EXIT_FAILURE);
	
	}	

	/*Condition block run by the parent since fork function returns the PID of the child to the parent process*/
	else if(return_fork > 0){
		/*sleep for 150 ms*/
		usleep(150000);

		fprintf(stderr, "%s%d\n", "Parent Process Execution. My Value ",my_value);

		/*create the message to be passed on to the child process*/
		sprintf(buffer, "Hi, I am your parent. My PID=%d and my_value=%d", getpid(), my_value);

		/*since the Parent proceess only sends a message to its child, close the read channel of the pipe*/
        	close(fd[0]);

        	/*write the message into the Write channel of the pipe*/
		write(fd[1],buffer,sizeof(buffer));	
		
        	/*wait for the child process to finish execution*/
		return_wait = wait(NULL);

		/*check for errors in execution of the child process*/
		if(return_wait == -1) {

			fprintf(stderr, "%s\n","Error in execution of wait function.");
			exit(EXIT_FAILURE);		
		}
		
		/*print the PID of the Parent and the Child*/
		fprintf(stderr, "%s%d\n", "Parent PID: ",getpid());
		fprintf(stderr, "%s%d\n", "Child PID: ",return_fork);

		/*close the write channel of the pipe*/
		close(fd[1]);
		exit(EXIT_SUCCESS);


	}
	/*
		Condition block run by the child process since, fork function returns 0 
		as the return value for function fork().
	*/
	else {

		/*sleep for 150 ms*/		
		usleep(150000);
		
		/*my_value is modified by the child process from 42 to a different value.*/
		my_value = 18951;

		/*print the child pid from within the child*/
		fprintf(stderr, "%s%d\n", "Child Process Execution. My Value ",my_value);
		fprintf(stderr, "%s%d\n", "From Child -> PID: ",getpid());
		
		/*since child only reads the message from parent, close the write channel of the pipe*/
		close(fd[1]);
		
		/*read the content of the read channel of the pipe*/
		read(fd[0],buffer,sizeof(buffer));

		/*sleep for 500 ms*/
		usleep(500000);

		/*print the received message*/
		fprintf(stderr,"Received message at child is %s\n",buffer);

		/*close the read channel of the pipe*/
		close(fd[0]);
		exit(EXIT_SUCCESS);
	}
}

int main() {

	task_3_1_function();
	return 0;
}
