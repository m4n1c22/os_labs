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
	printf("Usage: task -p <rate> <message> | task -c <rate>\n");
	exit(1);
	}

void produce(int argc, char **argv){
	int fd;
	int x=0;
	int i;

	if(argc != 4){
		print_usage();
		}		

	int rate = atoi(argv[2]);
	char msg[20];
	strcpy(msg,argv[3]);

	printf("rate: %d\nMessage: %s\n",rate,msg);

	while(x  < 2){
		fd = open("/dev/deeds_fifo", O_WRONLY);

		if(fd < 0){
			printf("Unable to open: \n");
			exit(1);
			}
		if(write(fd,&msg,strlen(msg))<0){
			perror("Unable to Write: \n");
			exit(1);
			}
		close(fd);
		x++;
		sleep(60/rate);
		}
	return;
	}



void consume(int argc, char **argv){
	int fd;
	int x=0;
	int i;

	if(argc != 3){
		print_usage();
		}

	int rate = atoi(argv[2]);
	char msg[20];
	int nbytes;

	printf("rate: %d\n",rate);

	while(x  < 2){
		fd = open("/dev/deeds_fifo", O_RDONLY);

		if(fd < 0){
			printf("Unable to open: \n");
			exit(1);
			}
		if(read(fd,&msg,nbytes)<0){
			perror("Unable to Write: \n");
			exit(1);
			}
		printf("%s\n",msg);
		close(fd);
		x++;
		sleep(60/rate);
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

