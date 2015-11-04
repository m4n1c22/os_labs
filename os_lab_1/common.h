//HEADERS

//Standard I/O Headers
#include <stdio.h>
#include <stdlib.h>

//String Headers
#include <string.h>

//File control Headers
#include <fcntl.h>

//Operating System related headers
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>		
#include <sys/types.h>
#include <unistd.h>

//Header for POSIX Threads	
#include <pthread.h>

//Header for Message Queues
#include <mqueue.h>


//Macros
#define BUFFER_SIZE 1024


#ifndef __COMMON_H__
#define __COMMON_H__

//Enumerations
typedef enum {
	SCENARIO_BEFORE_WAIT=0,
	SCENARIO_AFTER_WAIT=1
}eSCENARIO;		


#endif


