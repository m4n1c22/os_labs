###Objective

Solving the producer consumer problem.


####Executing task1_1.c

* Initially compile the task by running the Makefile with the command `sudo make`. Ensure that `obj-m+= task1_1.o` entry is present in the Makefile.
* Execute the command `insmod` to insert the module into the kernel. But since a parameter is passed into the mentioned module. The execution command needs to be `sudo insmod task1_1.ko fifo_size=123` where 123 is a parameter passed for the size of fifo allocated.
* Run the `fifo_perm.sh` script to provide Read/Write permissions to the driver file `/dev/deeds_fifo`. 
* Verification of module can be done running the `dmesg` command in terminal to determine if the module is loaded in the kernel or not.
* For inserting data use `sudo echo "1,1546664,helloworld" > /dev/deeds_fifo` command in the terminal, which writes data to the FIFO Queue created.
* For removing or displaying data from the queue run the command `cat /dev/deeds_fifo`.
* For removing the module from the kernel run the command `sudo rmmod task1_1`
* For checking stats run the command `cat /proc/deeds_fifo_stat`.

####Executing producer.c

* Initially compile the task by running the Makefile with the command `sudo make`. Ensure that `obj-m+= producer.o` entry is present in the Makefile.
* Execute the command `insmod` to insert the module into the kernel. But since a parameter is passed into the mentioned module. The execution command needs to be `sudo insmod task1_1.ko fifo_size=123` where 123 is a parameter passed for the size of fifo allocated.
* Run the `fifo_perm.sh` script to provide Read/Write permissions to the driver file `/dev/deeds_fifo`. 
* Verification of module can be done running the `dmesg` command in terminal to determine if the module is loaded in the kernel or not.
* Instead of inserting the data through the driver file `/dev/deeds_fifo` we are using producer LKM. Execute the 0`insmod producer.ko rate=3 item="1,1546664,helloworld"`.  `rate` and `item` are module parameters for producer module. Where former defines the rate of inflow and latter with inflow item in question. 
* Verification of module can be done running the `dmesg` command in terminal to determine if the module is loaded in the kernel or not. You can also view the data item in question in the terminal while running the above command for verification.
* For removing or displaying data from the queue run the command `cat /dev/deeds_fifo`.
* For removing the module from the kernel run the command `sudo rmmod task1_1`
* For checking stats run the command `cat /proc/deeds_fifo_stat`.

####Executing consumer.c

* Initially compile the task by running the Makefile with the command `sudo make`. Ensure that `obj-m+= consumer.o` entry is present in the Makefile.
* Execute the command `insmod` to insert the module into the kernel. But since a parameter is passed into the mentioned module. The execution command needs to be `sudo insmod task1_1.ko fifo_size=123` where 123 is a parameter passed for the size of fifo allocated.
* Run the `fifo_perm.sh` script to provide Read/Write permissions to the driver file `/dev/deeds_fifo`. 
* Verification of module can be done running the `dmesg` command in terminal to determine if the module is loaded in the kernel or not.
* For inserting data use `sudo echo "1,1546664,helloworld" > /dev/deeds_fifo` command in the terminal, which writes data to the FIFO Queue created.
* Instead of displaying or deleting the data through the driver file `/dev/deeds_fifo` we are using consumer LKM. Execute the `insmod consumer.ko rate=3"`.  `rate` is the module parameter for consumer module. The parameter deals with rate of outflow.
* Verification of module can be done running the `dmesg` command in terminal to determine if the module is loaded in the kernel or not. You can also view the data item in question in the terminal while running the above command for verification.
* For removing the module from the kernel run the command `sudo rmmod consumer.ko`
* For checking stats run the command `cat /proc/deeds_fifo_stat`.

*Note: Insertions can also be done using the combination of LKMs and /dev/deeds_fifo drivers*

Hope you have fun!!!
