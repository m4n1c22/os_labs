/**
	\file	:	task1_1.c
	\author	: 	Team Mango
	\brief	:	Task 1 of OS Lab-3 related to FIFO Queue implemented.
*/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/device.h>
#include <asm/uaccess.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/semaphore.h>
#include <linux/time.h>

MODULE_AUTHOR("Team Mango");
MODULE_DESCRIPTION("Lab Solution Task 1.1");
MODULE_LICENSE("GPL");

/** STANDARD MACROS */

#define BASE_10             10
#define END_OF_BUFF         '\0'

/** MEMORY ALLOCATION RELATED MACROS */

#define LOWER_LIMIT         4
#define UPPER_LIMIT         4096
#define DEFAULT_MEM_SIZE    32

#define IN_RANGE(MEM)       ((MEM>=LOWER_LIMIT)&&(MEM<=UPPER_LIMIT))

/** DEVICE RELATED MACROS */

#define FIFO_DEVICE        "deeds_fifo"
#define FIFO_DEVICE_NAME   "deeds_fifo"
#define FIFO_CONFIG        "deeds_fifo_stats"
#define MAJOR_NUM          250
#define MINOR_NUM_FIFO     0
#define CLASS_NAME         "fifo_class"

/** Proc FS File Object */
static struct proc_dir_entry *fifo_config_file_entry;

/** Device Class Object */
static struct class*  fifoClass  = NULL;

/** Device Object */
static struct device* fifo = NULL;

/** MEMORY RELATED VARIABLES */
static int mem_alloc_size;

/** Flags */
static int finished_config;
static int finished_fifo;

/** User Defined Data Item Structure */
struct data_item {
	unsigned int qid;
	unsigned long long time;
	char *msg;
};


/** FIFO_QUEUE */
static struct data_item *queue;

/** FIFO QUEUE Pointers*/
static int head;
static int tail;

/** Push and Pop counters*/
static int push;
static int pop;

/** Producer Consumer counters */
static int producer_ctr;
static int consumer_ctr;
static int user_level_producer_ctr;
static int user_level_consumer_ctr;

/** FIFO Statistics Variables */
static int num_items;
static int num_empty_slots;
static int fill_percentage;

/** Parameters passed to Module */
static int fifo_size;

/** Semaphores to producer & consumer problem */
static struct semaphore mutex;
static struct semaphore empty;
static struct semaphore full;

/** Semaphores for producer-consumer counters */
static struct semaphore producer_mutex;
static struct semaphore consumer_mutex;

static struct semaphore user_level_producer_mutex;
static struct semaphore user_level_consumer_mutex;

/** 
	Safe Incremental/Decremental Functions for all producers 
	and consumers.
*/
int producerInc(void);
int producerDec(void);
int consumerInc(void);
int consumerDec(void);

/** 
	Safe Incremental/Decremental Functions for user-level producers 
	and consumers.
*/
int userLevelProducerInc(void);
int userLevelProducerDec(void);
int userLevelConsumerInc(void);
int userLevelConsumerDec(void);

/** Basic FIFO Method Prototypes */
static ssize_t fifo_read(char *buf, size_t count, loff_t *ppos);
static ssize_t fifo_write(const char *buf, size_t count, loff_t *ppos);

/** Exporting Functions*/
EXPORT_SYMBOL_GPL(fifo_read);
EXPORT_SYMBOL_GPL(fifo_write);


/**
	Function Name : fifo_module_read
	Function Type : Kernel Callback Method
	Description   : Method is invoked whenever the fifo device files are
					read. This callback method is triggered when a read
					operation performed on the devices register to this
					file operation object. FIFO Devices are allocated.
*/
static ssize_t fifo_module_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
	/** return value storage variables */
	
	int ret; /** return value dealing with safe operation methods */
	int ret_fiforead; /** return value*/

	/** Performing a safe Increment of User Level Consumer counter */
	ret=userLevelConsumerInc();
	
	/** Verifying if Mutual exclusion inhibits the propagation.*/
	if (ret!=0) {
		/** Issue a restart of syscall which was supposed to be executed.*/
		return -ERESTARTSYS;
	}	
	
	/** Condition to check if the EOF is reached.*/
	if(finished_fifo) {
		/** Performing a safe Decrement of User Level Consumer counter*/
		ret=userLevelConsumerDec();
		/** Verifying if Mutual exclusion inhibits the propagation.*/
		if (ret!=0) {
			/** Issue a restart of syscall which was supposed to be executed.*/
			return -ERESTARTSYS;
		}
		/** Successful execution of read callback with EOF reached.*/
		return 0;
	}
	
	/** Flag set to Completed marking EOF.*/
	finished_fifo = 1;
	
	/** Calling the internal fifo_read() method call.*/
	ret_fiforead=fifo_read(buf,count,ppos);
	
	/** Performing a safe Decrement of User Level Consumer counter*/
	ret=userLevelConsumerDec();
	
	/** Verifying if Mutual exclusion inhibits the propagation.*/
	if (ret!=0) {
		/** Issue a restart of syscall which was supposed to be executed.*/
		return -ERESTARTSYS;
	}
	/** Successful execution of read callback with some bytes*/
	return ret_fiforead;
}

/**
	Function Name : fifo_read()
	Function Type : Kernel Internal Module Method
	Description   : Method is invoked internally by the 
					fifo_module_read() and externally from other kernel
					modules such as producer modules loaded.
*/
static ssize_t fifo_read(char *buf, size_t count, loff_t *ppos)
{
	/** return value storage variables */
	
	int ret; /** return value dealing with safe operation methods */
	int ret_buf;

	printk(KERN_INFO "FIFO:Fifo module is being read.\n");

	/** Performing a safe Increment of Consumer counter*/		
	ret=consumerInc();

	/** Verifying if Mutual exclusion inhibits the propagation.*/
	if (ret!=0) {
		/** Issue a restart of syscall which was supposed to be executed.*/
		return -ERESTARTSYS;
	}
	
	if (down_interruptible(&empty)){
		
		printk(KERN_ALERT "FIFO ERROR:Fifo Read access failed");
		/** Performing a safe Increment of Consumer counter*/		
		ret=consumerDec();
		/** Verifying if Mutual exclusion inhibits the propagation.*/
		if (ret!=0) {
			/** Issue a restart of syscall which was supposed to be executed.*/
			return -ERESTARTSYS;
		}
		/** Issue a restart of syscall which was supposed to be executed.*/
		return -ERESTARTSYS;
	}
	else {
		if(down_interruptible(&mutex)){
			printk(KERN_ALERT "FIFO ERROR:Mutual Exclusive position access failed from read module");
			up(&empty);
			/** Performing a safe Increment of Consumer counter*/		
			ret=consumerDec();
			/** Verifying if Mutual exclusion inhibits the propagation.*/
			if (ret!=0) {
				/** Issue a restart of syscall which was supposed to be executed.*/
				return -ERESTARTSYS;
			}
			/** Issue a restart of syscall which was supposed to be executed.*/
			return -ERESTARTSYS;
		}
		else {
			
			printk(KERN_INFO "FIFO:queue[head].msg = %s\n", queue[head].msg);
			ret_buf = sprintf(buf,"%d, %lld, %s",queue[head].qid, queue[head].time, queue[head].msg);
			if(ret_buf <0) {
				/** Memory allocation problem */
				return -ENOMEM;
			}
			

			kfree(queue[head].msg);
			if(head==(mem_alloc_size-1)) {
				head = 0;
			}
			else if(head==tail) {
				head = -1;
				tail = -1;
			}
			else {
				head = head+1;
			}
			pop = pop + 1;
			up(&mutex);
			up(&full);
			
			/** Performing a safe Increment of Consumer counter*/		
			ret=consumerDec();
			/** Verifying if Mutual exclusion inhibits the propagation.*/
			if (ret!=0) {
				/** Issue a restart of syscall which was supposed to be executed.*/
				return -ERESTARTSYS;
			}

			/** Successful execution of read callback with some bytes*/
			return ret_buf;
		}
	}
}

/**
	Function Name : fifo_module_write
	Function Type : Kernel Callback Method
	Description   : Method is invoked whenever the fifo device files are
			written. This callback method is triggered when a
			write operation performed on the devices register to
			this file operation object.
			FIFO Devices are allocated with one device being
			read only(FIFO1) and other being write only(FIFO0).
*/
static ssize_t fifo_module_write(struct file *file, const char *buf, size_t count, loff_t *ppos)
{	
	int ret;
	int ret_fifowrite;
	ret=userLevelProducerInc();
	if (ret!=0)
		return -ERESTARTSYS;
	
	ret_fifowrite= fifo_write(buf,count,ppos);

	ret=userLevelProducerDec();
	if (ret!=0)
		return -ERESTARTSYS;

	return ret_fifowrite;
}
static ssize_t fifo_write(const char *buf, size_t count, loff_t *ppos)
{
	int ret;
	struct timeval timeval_obj;
	printk(KERN_INFO "FIFO:head = %d, tail = %d", head,tail);

	ret=producerInc();
	if (ret!=0)
		return -ERESTARTSYS;

	if (( buf == NULL) ||  (*buf == 0)) {
		ret=producerDec();
		if (ret!=0)
			return -ERESTARTSYS;
      		return -ENOMEM;
	}

	if (down_interruptible(&full)){
		printk(KERN_ALERT "FIFO ERROR:Write access failed.");
		ret=producerDec();
		if (ret!=0)
			return -ERESTARTSYS;
		return -ERESTARTSYS;
	}
	else {
		if (down_interruptible(&mutex)){
			printk(KERN_ALERT "FIFO ERROR: Mutual Exclusive access failed from write module");
			up(&full);
			ret=producerDec();
			if (ret!=0)
				return -ERESTARTSYS;
			return -ERESTARTSYS;
		}
		else {

			
			if((head == -1)&&(tail==-1)) {
				head = 0;
			}
			else if(tail==mem_alloc_size-1) {
				tail=-1;
			}
			tail = tail+1;
			printk(KERN_INFO "FIFO:head = %d, tail = %d", head,tail);
			do_gettimeofday(&timeval_obj);
			queue[tail].qid = push;
			queue[tail].time = timeval_obj.tv_sec;
			queue[tail].msg = kmalloc(strlen(buf),GFP_KERNEL);
			ret=sprintf(queue[tail].msg,buf);
			
			if(ret<0) {
				up(&mutex);
				up(&full);
				ret=producerDec();
				if (ret!=0)
					return -ERESTARTSYS;				
				return ret;
			}
			printk(KERN_INFO "FIFO:Fifo module is being written.\n");


			push = push + 1;

			up(&mutex);
			up(&empty);
			ret=producerDec();
			if (ret!=0)
				return -ERESTARTSYS;
			/** Successful execution of write callback with buffer count.*/
			return count;
		}
	}
}


/**
	Function Name : fifo_module_open
	Function Type : Kernel Callback Method
	Description   : Method is invoked whenever the fifo device files are
			opened. This callback method is triggered when an
			open operation performed on the devices register to
			this file operation object. The open system call is
			invoked whenever an operation of read/write is
			performed on the device.
			FIFO Devices are allocated with one device being
			read only(FIFO1) and other being write only(FIFO0).
*/
static int fifo_module_open(struct inode * inode, struct file * file)
{
	

	/** Finished flag set to false indicating file is just opened.*/
	finished_fifo = 0;

	/** Successful execution of open callback */
	return 0;
}

/**
	Function Name : fifo_module_release
	Function Type : Kernel Callback Method
	Description   : Method is invoked whenever the fifo device files are
        	        closed. This callback method is triggered when a
                	close operation performed on the devices register to
        		this file operation object.
			FIFO Devices are allocated with one device being
			read only(FIFO1) and other being write only(FIFO0).
*/
static int fifo_module_release(struct inode * inode, struct file * file)
{
	printk(KERN_INFO "FIFO:Fifo module is being released.\n");

	/** Successful execution of release callback */
	return 0;
}


/**
	Function Name : fifo_config_module_read
	Function Type : Kernel Callback Method
	Description   : Method is invoked whenever the fifo_config file is
			read. This callback method is triggered when a read
			operation performed on the /proc/fifo_config.
			The /proc/fifo_config contains the information about
			the memory info	of the FIFO Queue. Such as allocated
			size, free size and total size.	The fifo_config is a
			RD/WR file. But can only be written if the queue is
			not in use.
*/
static ssize_t fifo_config_module_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
	int ret;
	printk(KERN_INFO "FIFO:Fifo config module is being read.\n");


        /** Condition to check if EOF is reached. */
	if(!finished_config){
		/** Flag set to Completed marking EOF.*/
		  finished_config = 1;

	if(head==-1 && tail==-1) {
		num_items=0;
		num_empty_slots=mem_alloc_size;
		fill_percentage=0;
		queue[head].qid=0;
		queue[tail].qid=0;
		
		}

	else {
  	num_items = tail - head + 1;
	num_empty_slots = mem_alloc_size - num_items;
	fill_percentage = (num_items*100)/mem_alloc_size;
	}
	ret = sprintf(buf,"Allocated Size: %d\nNumber of items stored: %d\nNumber of empty slots: %d\nPercentage of filled slots: %d \nNumber of insertions performed: %d\nNumber of removals performed: %d\nActive No.of Producers: %d\nActive No.of Consumers: %d\nActive User level Producers: %d\nActive User level Consumers: %d\nFirst Data Item Sequence No:%d\nLatest Data Item Sequence No:%d\n", mem_alloc_size,num_items,num_empty_slots,fill_percentage,push,pop,producer_ctr,consumer_ctr,user_level_producer_ctr,user_level_consumer_ctr,queue[head].qid,queue[tail].qid);

		if(ret < 0) {
			/** Memory allocation problem */
			return -ENOMEM;
			}
		/** Successful execution of read callback with some bytes*/
		return ret;
	}

	
	/** Successful execution of read callback with EOF reached.*/
	return 0;
}

/**
	Function Name : fifo_config_module_write
	Function Type : Kernel Callback Method
	Description   : Method is invoked whenever the fifo_config file is
			written. This callback method is triggered when a
			write operation performed on the /proc/fifo_config.
			The /proc/fifo_config contains the information
			about the memory info of the FIFO Queue. Such as
			allocated size, free size and total size. The
			fifo_config is a RD/WR file. But can only be written
			if the queue is not in use.
*/
static ssize_t fifo_config_module_write(struct file *file, const char *buf, size_t count, loff_t *ppos)
{
	printk(KERN_INFO "FIFO:Fifo config module is being written.\n");	
	/** Successful execution of write callback with buffer count.*/
	return count;
}

/**
	Function Name : fifo_config_module_open
	Function Type : Kernel Callback Method
	Description   : Method is invoked whenever the fifo_config file is
			opened. This callback method is triggered when an
			open operation performed on the	/proc/fifo_config.
			This can be triggered on calls for read/write
			operations on /proc/fifo_config file.
			The /proc/fifo_config contains the information about
			the memory info	of the FIFO Queue. Such as allocated
			size, free size and total size.	The fifo_config is a
			RD/WR file. But can only be written if the queue is
			not in use.
*/
static int fifo_config_module_open(struct inode * inode, struct file * file)
{
	printk(KERN_INFO "FIFO:Fifo config module is being opened.\n");

	/** Finished flag set to false indicating file is just opened.*/
	finished_config = 0;
	/** Successful execution of open callback. */
	return 0;
}

/**
	Function Name : fifo_config_module_release
	Function Type : Kernel Callback Method
	Description   : Method is invoked whenever the fifo_config file is
			closed. This callback method is triggered when a
			close operation performed on the /proc/fifo_config.
			This can be triggered on calls for read/write
			operations on /proc/fifo_config file.
			The /proc/fifo_config contains the information about
			the memory info	of the FIFO Queue. Such as allocated
			size, free size and total size.	The fifo_config is a
			RD/WR file. But can only be written if the queue is
			not in use.
*/
static int fifo_config_module_release(struct inode * inode, struct file * file)
{
	printk(KERN_INFO "FIFO:Fifo config module is being released.\n");
	/** Successful execution of release callback */
	return 0;
}

/**
    File Operations for handling the /proc/deeds_fifo_stat file accesses.
*/
static struct file_operations fifo_config_module_fops = {
    .owner   =	THIS_MODULE,
    .read    =	fifo_config_module_read,
    .write   =	fifo_config_module_write,
    .open    =	fifo_config_module_open,
    .release =	fifo_config_module_release,
};

/**
    File Operations for handling the fifo devices file accesses.
*/
static struct file_operations fifo_module_fops = {
    .owner   =	THIS_MODULE,
    .read    =	fifo_module_read,
    .write   =	fifo_module_write,
    .open    =	fifo_module_open,
    .release =	fifo_module_release,
};

/**
	Function Name : fifo_module_init
	Function Type : Module INIT
	Description   : Initialization method of the Kernel module. The
					method gets invoked when the kernel module is being
					inserted using the command insmod.
*/
static int __init fifo_module_init(void)
{
	int ret;
	printk(KERN_INFO "FIFO:FIFO module is being loaded.\n");

	/**Proc FS is created with RD ONLY permissions with name fifo_config*/
	fifo_config_file_entry = proc_create(FIFO_CONFIG,0644,NULL,&fifo_config_module_fops);

	/** Condition to verify if fifo_config creation was successful*/
	if(fifo_config_file_entry == NULL) {
		printk(KERN_ALERT "FIFO ERROR: Could not initialize /proc/%s\n",FIFO_CONFIG);
		/** FILE CREATION PROBLEM */
		return -ENOMEM;
	}

	/**
	    Registering the Device with a major number as 250 and
	    configuring the file operations associated with it.
	*/
	ret = register_chrdev(MAJOR_NUM, FIFO_DEVICE, &fifo_module_fops);

	/** Condition code to check if the registration was successful.*/
	if (ret < 0) {
		printk(KERN_ALERT "FIFO ERROR: %s failed with %d\n",
		       "Sorry, registering the character device ", MAJOR_NUM);
		/** Registration error.*/
		return ret;
	}
	printk(KERN_INFO "FIFO:registered correctly with major number %d\n", MAJOR_NUM);

    /** Registering device class and associating devices with it.*/
    fifoClass = class_create(THIS_MODULE, CLASS_NAME);

	/** Condition check if the class creation was successful. */
	if (IS_ERR(fifoClass)){
		/** Unregister the device due to failed class creation. */
		unregister_chrdev(MAJOR_NUM, FIFO_DEVICE);
		printk(KERN_ALERT "FIFO ERROR:Failed to register device class\n");

		/** Class creation error.*/
		return PTR_ERR(fifoClass);
	}
	printk(KERN_INFO "FIFO: device class registered correctly\n");

    	/**
            Registering the device driver for the provided device class. The
            device driver is associated with fifo0 with minor number as 0.
    	*/
	fifo = device_create(fifoClass, NULL, MKDEV(MAJOR_NUM, MINOR_NUM_FIFO), NULL, FIFO_DEVICE_NAME);

	/** Condition for error verification during driver creation.*/
	if (IS_ERR(fifo)){

		/** Class destroyed associated with the device drivers.*/
		class_destroy(fifoClass);
		/** Unregister the device due to failed driver creation. */
		unregister_chrdev(MAJOR_NUM, FIFO_DEVICE);
		printk(KERN_ALERT "FIFO ERROR:Failed to create the device\n");
		/** Driver creation error.*/
		return PTR_ERR(fifo);
	}
	printk(KERN_INFO "FIFO:device class created correctly\n");

	/** Default Memory size of queue set to 8*/
	mem_alloc_size = fifo_size;

	/** Queue Allocated with the default size.*/
	queue = (struct data_item*)kmalloc(mem_alloc_size*sizeof(struct data_item),GFP_KERNEL);

	/** Condition to check if the memory allocation was successful.*/
	if(!queue) {
		printk(KERN_ERR "FIFO ERROR:Memory allocation problem.\n");
		/** Memory allocation problem.*/
		return -ENOMEM;
	}

	/** FIFO HEAD Set to FIRST Location. */
	head = -1;
	tail = -1;

	/** Initializing push and pop counters*/

	push = 0;
	pop = 0;

	/** Initializing the semaphores */
	sema_init(&mutex,1);
	sema_init(&empty,0);
	sema_init(&full,mem_alloc_size);

	sema_init(&producer_mutex,1);
	sema_init(&consumer_mutex,1);

	sema_init(&user_level_producer_mutex,1);
	sema_init(&user_level_consumer_mutex,1);
	
	/** Initialize producer consumer counters */
	producer_ctr=0;
	consumer_ctr=0;

	user_level_producer_ctr=0;
	user_level_consumer_ctr=0;

	num_items = 0;
	num_empty_slots = fifo_size;
	fill_percentage = 0;

	/** Successful execution of initialization method. */
	return 0;
}

/**
	Function Name : fifo_module_cleanup
	Function Type : Module EXIT
	Description   : Cleanup method of the Kernel module. The
                	method gets invoked when the kernel module is being
                 	removed using the command rmmod.
*/
static void __exit fifo_module_cleanup(void)
{

	printk(KERN_INFO "FIFO:FIFO module is being unloaded.\n");

	/** Removing the Proc FS entry. */
	proc_remove(fifo_config_file_entry);

	/** Removing the device with minor number 0 => FIFO0 */
	device_destroy(fifoClass, MKDEV(MAJOR_NUM, MINOR_NUM_FIFO));
	/** Deregistering the class FIFO*/
	class_unregister(fifoClass);
	/** Deallocating the class FIFO*/
	class_destroy(fifoClass);
	/** Deregistering the character Device FIFO*/
	unregister_chrdev(MAJOR_NUM, FIFO_DEVICE);

	/** Deallocating the Queue */
	kfree(queue);
}


/**
	Function Name   :   queueAlloc
	Function Type   :   Custom
	Description     :   Method used to allocate/re-allocate the queue
                            from various module calls.
	Parameter       :   mem_size is used to allocate the memory for the
                            queue provided.
*/
int queueAlloc(int mem_size) {

	/** Condition check to see if the queue is already allocated.*/
	if(queue) {
		/** deallocating the queue.*/
		kfree(queue);
	}
	/** Allocating queue with the new mem_size.*/
	queue = (struct data_item*) kmalloc(mem_size*sizeof(struct data_item),GFP_KERNEL);
	/** Condition to see if the allocation was successful or not.*/
	if(!queue) {
		printk(KERN_ERR "FIFO ERROR:Memory allocation problem.\n");

		/** Memory allocation problem.*/
		return -ENOMEM;
	}
	/** Setting the memory allocation size */
	mem_alloc_size = mem_size;

	/** FIFO HEAD Set to FIRST Location. */
	head = -1;
	tail = -1;

	/** Successful execution of Queue Allocation method*/
	return 0;
}

/**
	Function Name   :   producerInc
	Function Type   :   Safe Increment
	Description     :   Method used to increment the producer counter
	 					variable but in a safe way with the use of
						Mutual Exclusion property.
	Return	        :   
*/

int producerInc(void) {

	if (down_interruptible(&producer_mutex)){
		
		printk(KERN_ALERT "FIFO ERROR:Producer counter Increment Mutex Failed");
		return -ERESTARTSYS;
	}

	producer_ctr++;
	up(&producer_mutex);
	return 0;
}

/**
	Function Name   :   producerDec
	Function Type   :   Safe Decrement
	Description     :   Method used to decrement the producer counter
	 					variable but in a safe way with the use of
						Mutual Exclusion property.
	Return	        :   
*/
int producerDec(void) {

	if (down_interruptible(&producer_mutex)){
		
		printk(KERN_ALERT "FIFO ERROR:Producer counter Decrement Mutex Failed");
		return -ERESTARTSYS;
	}

	producer_ctr--;
	up(&producer_mutex);
	return 0;
}

int consumerInc(void) {

	if (down_interruptible(&consumer_mutex)){
		
		printk(KERN_ALERT "FIFO ERROR:Consumer counter Increment Mutex Failed");
		return -ERESTARTSYS;
	}

	consumer_ctr++;
	up(&consumer_mutex);
	return 0;
}

int consumerDec(void) {

	if (down_interruptible(&consumer_mutex)){
		
		printk(KERN_ALERT "FIFO ERROR:Consumer counter Decrement Mutex Failed");
		return -ERESTARTSYS;
	}

	consumer_ctr--;
	up(&consumer_mutex);
	return 0;
}

int userLevelProducerInc(void) {

	if (down_interruptible(&user_level_producer_mutex)){
		
		printk(KERN_ALERT "FIFO ERROR:Producer counter Increment Mutex Failed");
		return -ERESTARTSYS;
	}

	user_level_producer_ctr++;
	up(&user_level_producer_mutex);
	return 0;
}

int userLevelProducerDec(void) {

	if (down_interruptible(&user_level_producer_mutex)){
		
		printk(KERN_ALERT "FIFO ERROR:Producer counter Decrement Mutex Failed");
		return -ERESTARTSYS;
	}

	user_level_producer_ctr--;
	up(&user_level_producer_mutex);
	return 0;
}

int userLevelConsumerInc(void) {

	if (down_interruptible(&user_level_consumer_mutex)){
		
		printk(KERN_ALERT "FIFO ERROR:Consumer counter Increment Mutex Failed");
		return -ERESTARTSYS;
	}

	user_level_consumer_ctr++;
	up(&user_level_consumer_mutex);
	return 0;
}

int userLevelConsumerDec(void) {

	if (down_interruptible(&user_level_consumer_mutex)){
		
		printk(KERN_ALERT "FIFO ERROR:Consumer counter Decrement Mutex Failed");
		return -ERESTARTSYS;
	}

	user_level_consumer_ctr--;
	up(&user_level_consumer_mutex);
	return 0;
}
/** Initializing the kernel module init with custom init method */
module_init(fifo_module_init);
/** Initializing the kernel module exit with custom cleanup method */
module_exit(fifo_module_cleanup);
/** Macro which deals with setting up of parameter passing to the module */
module_param(fifo_size,int,0);
