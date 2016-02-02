###Objective

Solving the producer consumer problem.


####Executing fifo.c

* Initially compile the task by running the Makefile with the command `sudo make`. Ensure that `obj-m+= fifo.o` entry is present in the Makefile.
* Execute the command `insmod` to insert the module into the kernel. But since a parameter is passed into the mentioned module. The execution command needs to be `sudo insmod fifo.ko fifo_size=123` where 123 is a parameter passed for the size of fifo allocated.
* Run the `fifo_perm.sh` script to provide Read/Write permissions to the driver file `/dev/deeds_fifo`. 
* Verification of module can be done running the `dmesg` command in terminal to determine if the module is loaded in the kernel or not.
* For inserting data use `sudo echo "helloworld" > /dev/deeds_fifo` command in the terminal, which writes data to the FIFO Queue created.
* For removing or displaying data from the queue run the command `cat /dev/deeds_fifo`.
* For removing the module from the kernel run the command `sudo rmmod fifo.ko`
* For checking stats run the command `cat /proc/deeds_fifo_stats`.

####Executing producer.c

* Initially compile the task by running the Makefile with the command `sudo make`. Ensure that `obj-m+= producer.o` entry is present in the Makefile.
* Execute the command `insmod` to insert the module into the kernel. But since a parameter is passed into the mentioned module. The execution command needs to be `sudo insmod fifo.ko fifo_size=123` where 123 is a parameter passed for the size of fifo allocated.
* Run the `fifo_perm.sh` script to provide Read/Write permissions to the driver file `/dev/deeds_fifo`. 
* Verification of module can be done running the `dmesg` command in terminal to determine if the module is loaded in the kernel or not.
* Instead of inserting the data through the driver file `/dev/deeds_fifo` we are using producer LKM. Execute the 0`insmod producer.ko rate=3 msg="helloworld" instance="producer_1"`.  `rate` defines the rate of inflow of data items , `msg` is the message stored in the data item in quesiton and `instance` is the identification name of the producer module executing the LKM. 
* Verification of module can be done running the `dmesg` command in terminal to determine if the module is loaded in the kernel or not. You can also view the data item in question in the terminal while running the above command for verification.
* For removing or displaying data from the queue run the command `cat /dev/deeds_fifo`.
* For removing the fifo module from the kernel run the command `sudo rmmod fifo.ko`
* For removing the producer module from the kernel run the command `sudo rmmod producer.ko`
* For checking stats run the command `cat /proc/deeds_fifo_stats`.

####Executing consumer.c

* Initially compile the task by running the Makefile with the command `sudo make`. Ensure that `obj-m+= consumer.o` entry is present in the Makefile.
* Execute the command `insmod` to insert the module into the kernel. But since a parameter is passed into the mentioned module. The execution command needs to be `sudo insmod fifo.ko fifo_size=123` where 123 is a parameter passed for the size of fifo allocated.
* Run the `fifo_perm.sh` script to provide Read/Write permissions to the driver file `/dev/deeds_fifo`. 
* Verification of module can be done running the `dmesg` command in terminal to determine if the module is loaded in the kernel or not.
* For inserting data use `sudo echo "helloworld" > /dev/deeds_fifo` command in the terminal, which writes data to the FIFO Queue created.
* Instead of displaying or deleting the data through the driver file `/dev/deeds_fifo` we are using consumer LKM. Execute the `insmod consumer.ko rate=3 instance="consumer_1"`.  `rate` is the module parameter for consumer module. The parameter deals with rate of outflow. `instance` is the identification name of the consumer module executing the LKM. 
* Verification of module can be done running the `dmesg` command in terminal to determine if the module is loaded in the kernel or not. You can also view the data item in question in the terminal while running the above command for verification.
* For removing the fifo module from the kernel run the command `sudo rmmod fifo.ko`
* For removing the consumer module from the kernel run the command `sudo rmmod consumer.ko`
* For checking stats run the command `cat /proc/deeds_fifo_stats`.

*Note: Insertions and deletions can also be done using the combination of LKMs and /dev/deeds_fifo drivers*

####Executing pr_cs.sh

*`pr_cs.sh` is a shell script which acts as a user level producer/consumer with a provision to provide stats of queue in use based on the parameter passed.
* Usage is `sh pr_cs.sh p 2 msg prod_1` where `p` is mode as in producer mode, `2` is rate of inflow, `msg` is the message and `prod_1` is the instance name.
* Usage is `sh pr_cs.sh c 2 consumer_1` where `c` is mode as in consumer mode, `2` is rate of outflow and `consumer_1` is the instance name.
* Usage is `sh pr_cs.sh stats 2` where `stats` is mode as in FIFO Stats mode, `2` is sleep rate or how frequent the stats data is accessed.
 
####Executing task2_3.sh

*`task2_3.sh` is a shell script which automates the user level producers  & consumers.
* Usage is `sh task2_3.sh`.

*Note: Before running the shell scripts ensure that you are in SuperUser mode.*
Hope you have fun!!!
