/**
	\file	:	task2_2.c
	\author	: 	Team Mango
	\brief	:	Task 2.2 of OS Lab-3 related to User level producers and consumers are implemented.
*/
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <mtd/mtd-user.h>
#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void print_usage(){
	printf("Usage: task -p <rate> <message> <instance_name>| task -c <rate> <instance_name>\n");
	exit(1);
	}

void produce(int argc, char **argv){
	int fd;
	int x=0;
	int i;

	if(argc != 5){
		print_usage();
		}		

	int rate = atoi(argv[2]);
	char instance[20];
	char msg[20];
	strcpy(msg,argv[3]);
	strcpy(instance,argv[4]);

	while(x  < 10){
		fd = open("/dev/deeds_fifo", O_WRONLY);

		if(fd < 0){
			printf("Unable to open: \n");
			exit(1);
			}
		if(write(fd,&msg,strlen(msg))<0){
			perror("Unable to Write: \n");
			exit(1);
			}
		printf("%s inserted message %s\n",instance,msg);
		close(fd);
		x++;
		sleep(1/rate);
		}
	return;
	}



void consume(int argc, char **argv){
	int fd;
	int x=0;
	int i;

	if(argc != 4){
		print_usage();
		}

	int rate = atoi(argv[2]);
	char msg[20];
	char instance[20];
	int nbytes;
	strcpy(instance,argv[3]);
	
	while(x  < 10){
		fd = open("/dev/deeds_fifo", O_RDONLY);

		if(fd < 0){
			printf("Unable to open: \n");
			exit(1);
			}
		if(read(fd,&msg,nbytes)<0){
			perror("Unable to Write: \n");
			exit(1);
			}
		printf("%s consumed message %s\n",instance,msg);
		close(fd);
		x++;
		sleep(1/rate);
		}

	return;
	}


int main (int argc, char ** argv){
	int i;
	
	if (strcmp(argv[1],"-p")==0){
				printf("This is a Producer\n");
				produce(argc,argv);
				} 
	else if (strcmp(argv[1],"-c")==0){
				printf("This is a Consumer\n");
				consume(argc,argv);
				}

	return 0;
	}

