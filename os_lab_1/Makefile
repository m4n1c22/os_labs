#MACROS
CC := gcc
ALL_WARNINGS := -Wall
PTHREAD_LIB := -lpthread
REAL_TIME_LIB := -lrt
OUTPUT_FILE_OPTION := -o

#Target option for Compiling all the tasks
all: make_task1_1 make_task2_1 make_task3_1 make_task4_1 make_task5_1

#Compiling Task1_1 and generating its respective output file. This target option is selected if only one file compilation is needed.
make_task1_1: task1_1.c 
		   	  $(CC) $(ALL_WARNINGS) $(OUTPUT_FILE_OPTION) task1_1.exe task1_1.c
#Compiling Task2_1 and generating its respective output file. This target option is selected if only one file compilation is needed.		   	  
make_task2_1: task2_1.c 
		   	  $(CC) $(ALL_WARNINGS) $(OUTPUT_FILE_OPTION) task2_1.exe task2_1.c $(PTHREAD_LIB)
#Compiling Task3_1 and generating its respective output file. This target option is selected if only one file compilation is needed.		   	  
make_task3_1: task3_1.c 
		   	  $(CC) $(ALL_WARNINGS) $(OUTPUT_FILE_OPTION) task3_1.exe task3_1.c
#Compiling Task4_1 and generating its respective output file. This target option is selected if only one file compilation is needed.		   	  
make_task4_1: task4_1.c
			  $(CC) $(ALL_WARNINGS) $(OUTPUT_FILE_OPTION) task4_1.exe task4_1.c $(REAL_TIME_LIB)
#Compiling Task5_1 and generating its respective output file. This target option is selected if only one file compilation is needed.			  
make_task5_1: task5_1.c 
		   	  $(CC) $(ALL_WARNINGS) $(OUTPUT_FILE_OPTION) task5_1.exe task5_1.c $(REAL_TIME_LIB)

#Target option for Executing all the tasks
run: run_task1_1 run_task2_1 run_task3_1 run_task4_1 run_task5_1
#Target option for Executing task1_1	
run_task1_1: task1_1.exe
		./task1_1.exe
#Target option for Executing task2_1				   	  
run_task2_1: task2_1.exe
		./task2_1.exe	
#Target option for Executing task3_1
run_task3_1: task3_1.exe
		./task3_1.exe
#Target option for Executing task4_1
run_task4_1: task4_1.exe
		./task4_1.exe
#Target option for Executing task5_1
run_task5_1: task5_1.exe
		./task5_1.exe
#Target option for Cleaning up the execution files
clean: 
	rm task1_1.exe task2_1.exe task3_1.exe task4_1.exe task5_1.exe
.PHONY: all run_task1_1 run_task2_1 run_task3_1 run_task4_1 run_task5_1
