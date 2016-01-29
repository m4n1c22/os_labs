###Objective

Solving the producer consumer problem.


####Executing task1_1.c

* Initially compile the task by running the Makefile with the command `sudo make`. Ensure that `obj-m:= task1_1.o` entry is present in the Makefile.
* Execute the command `insmod` to insert the module into the kernel. But since a parameter is passed into the mentioned module. The execution command needs to be `sudo insmod task1_1.ko fifo_size=123` where 123 is a parameter passed for the size of fifo allocated.
* Run the `fifo_perm.sh` script to provide Read/Write permissions to the driver file `/dev/deeds_fifo`. 
* Verification of module can be done running the `dmesg` command in terminal to determine if the module is loaded in the kernel or not.
* For inserting data use `sudo echo "data" > /dev/deeds_fifo` command in the terminal, which writes data to the FIFO Queue created.
* For removing or displaying data from the queue run the command `cat /dev/deeds_fifo`.
* For removing the module from the kernel run the command `sudo rmmod task1_1`
* For checking stats run the command `sudo cat /proc/fifo_config`.
