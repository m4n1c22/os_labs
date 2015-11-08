
#include "common.h"

//Global variable generated for the processes.
int my_value = 42;


void task_4_1_function() {

	pid_t return_fork;
	pid_t return_wait;
	
	mqd_t mqds, mqdc;
	int mqsend;
	//char *sndmsg; char snd[256];
	char rcvmsg[256];
	int status; 
	int returnstatus;
	
	//msg queue attributes initialisation	
	struct mq_attr mqAttr;
	mqAttr.mq_flags=0;
	mqAttr.mq_maxmsg = 10;
	mqAttr.mq_msgsize = 20	;
	mqAttr.mq_curmsgs=0;
		

	return_fork = fork();
	
	// fork() error
	if(return_fork<0) {
		
		fprintf(stderr, "%s\n","Process not forked.");
		exit(EXIT_FAILURE);
	
	}	
	
	//parent process
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

		//fprintf(stderr, "%s%s\n", "Send Message:", sndmsg);
			
			if(mqsend==-1) 
			{
			fprintf(stderr, "%s\n","Error in Message Sending to Queue");
			exit(EXIT_FAILURE);		
			}
			
			fprintf(stderr, "%s\n","Message sent to queue."); 

			return_wait = wait(NULL);
			if(return_wait == -1) 
			{
			fprintf(stderr, "%s\n","Error in execution of wait function.");
			exit(EXIT_FAILURE);
			}

		//queue closed by parent,returns 0 if OK, -1 on error		
		returnstatus=mq_close (mqds);
		if(returnstatus==-1)
		{
		fprintf(stderr, "%s\n","Error in Message Queue closing by Parent.");
				       exit(EXIT_FAILURE); 
			
		}
		
		fprintf(stderr, "%s\n","Message Queue closed by Parent."); 

		//queue deletion by parent,returns 0 if OK, -1 on error
		returnstatus=mq_unlink("/DEEDS_lab1_mq");
		if(returnstatus==-1) 
		{ 
		fprintf(stderr, "%s\n","Error in Message Queue Deletion by Parent.");
				       exit(EXIT_FAILURE); 
		
		}
		fprintf(stderr, "%s\n","Message Queue Deleted by Parent."); 
	
		
	}

	//child process
	else {

		//The process sleeps for 150ms.
		usleep(150000);
		
		//my_value is modified by the child process from 42 to a different value.
		my_value = 18951;
		
		fprintf(stderr, "%s%d\n", "Child Process Execution. My Value:",my_value);

		//open the queue for reading; returns queue descriptor, -1 on error		
		mqdc = mq_open("/DEEDS_lab1_mq", O_RDONLY);
		if(mqdc== -1) 
		{
		fprintf(stderr, "%s\n","Error in Message Queue Creation.");
			exit(EXIT_FAILURE);
		}
		fprintf(stderr, "%s\n","Queue opened for reading."); 
		//receive message from queue, returns length of received message, -1 on error
		status = mq_receive(mqdc, rcvmsg, sizeof(rcvmsg), NULL);

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
