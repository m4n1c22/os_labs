####OS LAB

####Objective:
Understanding various Operating Systems Kernel & Device Driver fundamentals.

####Code 
#####Structure
The folder contains: 
- Makefile 
- task1_1.c - Implementation of task1_1. 
- task1_2.c - Implementation of task1_2. 
- task2_1.c - Implementation of task2_1. 

#####Build & Run
 
Makefile is meant to build the various tasks in the given folder.
For compiling the tasks use the command ```make```. 
If you run ```make``` without target, it will build all the targets.
Thus, compiling all tasks.

######Steps to run Task 1.2
 - ```make```
 - ```sudo insmod task1_2.ko```
 - ```echo "1" > /proc/deeds_clock_config or cat>/proc/deeds_clock_config 1 ctrl+D```
 - ```cat /proc/deeds_clock```
 - ```echo "0" > /proc/deeds_clock_config or cat>/proc/deeds_clock_config 0 ctrl+D```

######Steps to Run Task 2.1
 - ```make```
 - ```sudo insmode task2_1.ko```
 - ```sudo sh fifo_perm.sh```
 - ```sudo echo 4 > /proc/fifo_config```
 - ```sudo echo "123" > /dev/fifo0```
 - ```cat /dev/fifo1```
