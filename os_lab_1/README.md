#### OS LAB

#### Objective:
Understanding various Operating Systems fundamentals such Processes, Threads and IPC.

#### Code 
##### Structure
The folder contains: 
- Makefile 
- task1_1.c - Implementation of task1_1. Understanding the implementation of ***fork()***
- task2_1.c - Implementation of task2_1. Understanding the implementation of POSIX Threads (PThreads)
- task3_1.c - Implementation of task3_1. Understanding the implementation of Pipes (IPC)
- task4_1.c - Implementation of task4_1. Understanding the implementation of Message Queues (IPC)
- task5_1.c - Implementation of task5_1. Understanding the implementation of Shared Memory (IPC) 
- common.h  - All the common headerfiles, macros, enumerations used in the tasks.
 
##### Build & Run
 
Makefile is meant to build the various tasks in the given folder.
For compiling the tasks use the command ```make```. 
If you run ```make``` without target, it will build all the targets.
Thus, compiling all tasks.
If you need to build a specific file. Execute the command ```make make_task1_1```, for building the task1_1
Similarly after compiling, you can execute the program by running the command ```make run``` for running all tasks.
And also you run task specific. You can run the command ```make run_task1_1``` for executing the task1_1
