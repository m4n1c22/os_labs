all: make_task1_1 make_task2_1 make_task3_1 make_task4_1 make_task5_1

make_task1_1: task1_1.c 
		   	  gcc -Wall -o task1_1.exe task1_1.c
make_task2_1: task2_1.c 
		   	  gcc -Wall -o task2_1.exe task2_1.c -lpthread
make_task3_1: task3_1.c 
		   	  gcc -Wall -o task3_1.exe task3_1.c
make_task4_1: task4_1.c
			  gcc -Wall -o task4_1.exe task4_1.c -lrt
make_task5_1: task5_1.c 
		   	  gcc -Wall -o task5_1.exe task5_1.c -lrt

run: run_task1_1 run_task2_1 run_task3_1 run_task4_1 run_task5_1
run_task1_1: task1_1.exe
		./task1_1.exe		   	  
run_task2_1: task2_1.exe
		./task2_1.exe	
run_task3_1: task3_1.exe
		./task3_1.exe
run_task4_1: task4_1.exe
		./task4_1.exe
run_task5_1: task5_1.exe
		./task5_1.exe
clean: 
	rm task1_1.exe task2_1.exe task3_1.exe task4_1.exe task5_1.exe
.PHONY: all run_task1_1 run_task2_1 run_task3_1 run_task4_1 run_task5_1
