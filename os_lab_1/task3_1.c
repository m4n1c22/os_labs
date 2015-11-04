
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
        
	

	

	if(pipe(fd)==-1){
	fprintf(stderr,"Pipe creation failed\n");	
	}
	else{
	fprintf(stderr,"pipe creation successful\n");
	}

	
	return_fork = fork();

	if(return_fork<0) {
		
		fprintf(stderr, "%s\n","Error in execution of fork function.");
		exit(EXIT_FAILURE);
	
	}	

	
	else if(return_fork > 0){
		
		usleep(150000);
		fprintf(stderr, "%s%d\n", "Parent Process Execution. My Value\n ",my_value);
		
		sprintf(buffer, "Hi, I am your parent. My PID=%d and my_value=%d", getpid(), my_value);

        	close(fd[0]);

        	
		write(fd[1],buffer,sizeof(buffer));	
		
        	
		return_wait = wait(NULL);

		
		if(return_wait == -1) {

			fprintf(stderr, "%s\n","Error in execution of wait function.");
			exit(EXIT_FAILURE);		
		}

		fprintf(stderr, "%s%d\n", "Parent PID: ",getpid());
		fprintf(stderr, "%s%d\n", "Child PID: ",return_fork);
		close(fd[1]);
		exit(EXIT_SUCCESS);


	}
	/*
		Condition block run by the child process since, fork function returns 0 
		as the return value for function fork().
	*/
	else {

		
		usleep(150000);
		
		
		close(fd[1]);
		

		read(fd[0],buffer,sizeof(buffer));

		usleep(500000);
		fprintf(stderr,"Received message at child is %s\n",buffer);

	
		close(fd[0]);
		exit(EXIT_SUCCESS);
	}
}

int main() {

	task_3_1_function();
	return 0;
}
