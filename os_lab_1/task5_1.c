/**
		\file 			:		task5_1.c
		\author			:		Sreeram Sadasivam
		\brief			: 		Understanding the implementation of shared memory in operating system.
*/

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

//Macros
#define BUFFER_SIZE 1024


//Global variable generated for the processes.
int my_value = 42;

/**
		\brief:	Function which implements the task 5.1. Implements IPC with Shared Memory
				
*/
void task_5_1_function() {

	char *buffer;
	
	int fd;

	pid_t return_fork;
	pid_t return_wait;
        
	fd = shm_open ( "/DEEDS_lab1_shm" , O_CREAT | O_EXCL | O_RDWR,S_IRUSR | S_IWUSR);
    ftruncate (fd, BUFFER_SIZE);
    addr = mmap ( NULL , BUFFER_SIZE, PROT_READ | PROT_WRITE,MAP_SHARED , fd, 0);

	

/*	
	if(pipe(fd)==-1){
	fprintf(stderr,"Pipe creation failed\n");	
	}
	else{
	fprintf(stderr,"pipe creation successful\n");
	}
*/
	
	return_fork = fork();

	if(return_fork<0) {
		
		fprintf(stderr, "%s\n","Error in execution of fork function.");
		exit(EXIT_FAILURE);
	
	}	

	
	else if(return_fork > 0){
		
		usleep(150000);



		fprintf(stderr, "%s%d\n", "Parent Process Execution. My Value\n ",my_value);
		
		snprintf(buffer, sizeof(buffer), "Hi, I am your parent. My PID=%d and my_value=%d", getpid(), my_value);

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

	task_5_1_function();
	return 0;
}
