
#include "common.h"

//Global variable generated for the processes.
int my_value = 42;

//Function implementing task 4.1 POSIX Message Queues
void task_4_1_function() {

	pid_t return_fork;
	pid_t return_wait;
	
	//message queue descriptor
	mqd_t mqds, mqdc;
	int mqsend;
	char rcvmsg[256];
	int status; 
	int returnstatus;
	
	//msg queue attributes initialisation	
	struct mq_attr mqAttr;
	mqAttr.mq_flags=0;
	mqAttr.mq_maxmsg = 10;
	mqAttr.mq_msgsize = 20	;
	mqAttr.mq_curmsgs=0;
		
	/*fork a child process*/
	return_fork = fork();
	
	// check if there is any error with fork process
	if(return_fork<0) 
	{
		fprintf(stderr, "%s\n","Process not forked.");
		exit(EXIT_FAILURE);
	}	
	
	//parent process block as fork function returns the PID of the child to the parent process
	else if (return_fork>0)
	{
		//The process sleeps for 150ms.
		usleep(150000);
		
		fprintf(stderr, "%s%d\n", "Parent Process Execution. My Value:",my_value);
		
		//create message queue, returns queue descriptor, -1 on error
		mqds = mq_open("/DEEDS_lab1_mq", O_CREAT|O_RDWR, 0644, &mqAttr); 

			//Error handling for the Queue Creation.
			if(mqds == -1) {
			fprintf(stderr, "%s\n","Error in Message Queue Creation.");
			exit(EXIT_FAILURE);		
			}
		
		fprintf(stderr, "%s\n","Message Queue Created."); 
		
		//open the queue for writing
		mqds = mq_open("/DEEDS_lab1_mq", O_WRONLY);
		
		//add the message to the queue, returns 0 if OK, -1 on error		
		mqsend=mq_send(mqds, "Hi I am your Parent", strlen("Hi I am your Parent")+1, 1);

			//error handling for message sending to queue
			if(mqsend==-1) 
			{
			fprintf(stderr, "%s\n","Error in Message Sending to Queue");
			exit(EXIT_FAILURE);		
			}
			
		fprintf(stderr, "%s\n","Message sent to queue."); 
		
		//parent waits for the child process to finish execution
		return_wait = wait(NULL);
			
			//check for errors in execution of the child process
			if(return_wait == -1) 
			{
			fprintf(stderr, "%s\n","Error in execution of wait function.");
			exit(EXIT_FAILURE);
			}

		//queue closed by parent,returns 0 if OK, -1 on error		
		returnstatus=mq_close (mqds);
		
			//check if there is error in closing the queue
			if(returnstatus==-1)
			{
			fprintf(stderr, "%s\n","Error in Message Queue closing by Parent.");
			exit(EXIT_FAILURE); 
			}
		
		fprintf(stderr, "%s\n","Message Queue closed by Parent."); 

		//queue deletion by parent,returns 0 if OK, -1 on error
		returnstatus=mq_unlink("/DEEDS_lab1_mq");
			
			//check if there is any error in queue deletion
			if(returnstatus==-1) 
			{ 
			fprintf(stderr, "%s\n","Error in Message Queue Deletion by Parent.");
			exit(EXIT_FAILURE); 
			}
		
		fprintf(stderr, "%s\n","Message Queue Deleted by Parent."); 
	}

	//child process, fork() function returns 0 as the return value 
	else {
		//child process sleeps for 150ms.
		usleep(150000);
		
		//my_value is modified by the child process from 42 to a different value.
		my_value = 18951;
		
		fprintf(stderr, "%s%d\n", "Child Process Execution. My Value:",my_value);

		//open the queue for reading; returns queue descriptor, -1 on error		
		mqdc = mq_open("/DEEDS_lab1_mq", O_RDONLY);
		
			//check for errors in opening the queue for reading
			if(mqdc== -1) 
			{
			fprintf(stderr, "%s\n","Error in Message Queue Creation.");
			exit(EXIT_FAILURE);
			}
			
		fprintf(stderr, "%s\n","Queue opened for reading."); 
		
		//receive message from queue, returns length of received message, -1 on error
		status = mq_receive(mqdc, rcvmsg, sizeof(rcvmsg), NULL);
			
			//check for errors in receiving message from queue
			if(status==-1) 
			{
			fprintf(stderr, "%s\n","Error in Receiving Message from Queue.");
			exit(EXIT_FAILURE);
			}
			
		//The child process sleeps for 500ms.
		usleep(500000);
		
		fprintf(stderr, "%s%s\n","Received Message from Queue: ", rcvmsg);
			
		// close queue; returns: 0 if OK, -1 on error
		status=mq_close(mqdc);
			 	
			 	//check for errors in closing the queue
				if(status==-1) 
				{ 
				fprintf(stderr, "%s\n","Error in Message Queue Closing.");
				exit(EXIT_FAILURE);
				}
				
		fprintf(stderr, "%s\n","Queue closed by child."); 
		exit(EXIT_SUCCESS);
	}
}

int main() {

	task_4_1_function();
	return 0;
}
