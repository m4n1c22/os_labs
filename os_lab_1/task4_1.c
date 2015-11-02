#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <mqueue.h>
#include <string.h>

//Global variable generated for the processes.
int my_value = 42;


void task_3_1_function() {

	pid_t return_fork;
	
	mqd_t mqd;
	int mqsend;
	//char *sndmsg;
	char rcvmsg[200];
	int status;
	
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
		
		//create message queue, returns queue descriptor, -1 on error
		mqd = mq_open("/DEEDS_lab1_mq", O_CREAT|O_RDWR, 0644, &mqAttr); 

		//Error handling for the Queue Creation.
		if(mqd == -1) {

			fprintf(stderr, "%s\n","Error in Message Queue Creation.");
			exit(EXIT_FAILURE);		
		}
		else
		{  
		fprintf(stderr, "%s\n","Message Queue Created."); 
		
		//open the queue for writing
		mqd = mq_open("/DEEDS_lab1_mq", O_WRONLY);
		
		//sndmsg="Hi I am your Parent. My Pid= and myvalue=";
		//add the message to the queue, returns 0 if OK, -1 on error		
		mqsend=mq_send(mqd, "Hi I am your Parent", strlen("Hi I am your Parent")+1, 1);
			
			if(mqsend==-1) 
			{
			fprintf(stderr, "%s\n","Error in Message Sennding to Queue");
			exit(EXIT_FAILURE);		
			}
			else
			{
			fprintf(stderr, "%s\n","Message sent to queue."); 
			}
		}
	}

	//child process
	else {

		//The process sleeps for 150ms.
		usleep(150000);
		
		//my_value is modified by the child process from 42 to a different value.
		my_value = 18951;
		
		//open the queue for reading; returns queue descriptor, -1 on error		
		mqd = mq_open("/DEEDS_lab1_mq", O_RDONLY);
		if(mqd== -1) 
		{
		fprintf(stderr, "%s\n","Error in Message Queue Creation.");
			exit(EXIT_FAILURE);
		}
		else{ fprintf(stderr, "%s\n","Queue opened for reading."); 
		//receive message from queue, returns length of received message, -1 on error
		status = mq_receive(mqd, rcvmsg, sizeof(rcvmsg), NULL);

			if(status==-1) {
			fprintf(stderr, "%s\n","Error in Message Queue Creation.");
			exit(EXIT_FAILURE);
			}
			else{ 
			fprintf(stderr, "%s%s\n","   Received Message from Queue: ", rcvmsg);
			// close queue; returns: 0 if OK, -1 on error
			status=mq_close(mqd);
			 	
				if(mqd==-1) { fprintf(stderr, "%s\n","Error in Message Queue Creation.");
				exit(EXIT_FAILURE); }
				else { fprintf(stderr, "%s\n","Queue closed by child."); }
			}
			
		}
		
		//The child process sleeps for 500ms.
		usleep(500000);
	}
}

int main() {

	task_3_1_function();
	return 0;
}
