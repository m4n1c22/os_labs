all: make_task1_1 make_task2_1 make_task3_1

make_task1_1: task1_1.c 
		   	  gcc -Wall -o task1_1.exe task1_1.c
make_task2_1: task2_1.c 
		   	  gcc -Wall -o task2_1.exe task2_1.c -lpthread
make_task3_1: task3_1.c 
		   	  gcc -Wall -o task3_1.exe task3_1.c
run: run_task1_1 run_task2_1 run_task3_1
run_task1_1: task1_1.exe
		./task1_1.exe		   	  
run_task2_1: task2_1.exe
		./task2_1.exe	
run_task3_1: task3_1.exe
		./task3_1.exe
clean: 
	rm task1_1.exe task2_1.exe task3_1.exe
.PHONY: all run_task1_1
