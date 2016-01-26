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

MODULE_AUTHOR("Team Mango");
MODULE_DESCRIPTION("Lab Solution Task 1.1");
MODULE_LICENSE("GPL");

/** STANDARD MACROS */

#define BASE_10             10
#define END_OF_BUFF         '\0'

/** MEMORY ALLOCATION RELATED MACROS */

#define LOWER_LIMIT         4
#define UPPER_LIMIT         4096
#define DEFAULT_MEM_SIZE    8

#define IN_RANGE(MEM)       ((MEM>=LOWER_LIMIT)&&(MEM<=UPPER_LIMIT))

/** DEVICE RELATED Macros */

#define FIFO_DEVICE        "deeds_fifo"
#define FIFO_DEVICE_NAME   "deeds_fifo"
#define FIFO_CONFIG        "fifo_config"
#define MAJOR_NUM          250
#define MINOR_NUM_FIFO     0
#define CLASS_NAME         "fifo_class"
#define IS_MINOR(A,B)      (A==B)

/** Proc FS File Object */
static struct proc_dir_entry *fifo_config_file_entry;

/** Device Class Object */
static struct class*  fifoClass  = NULL; 

/** Device Object */
static struct device* fifo = NULL; 

/** FIFO_QUEUE */
static char *queue;

/** MEMORY RELATED VARIABLES */
static int mem_alloc_size;

/** Flags */
static int finished_config;
static int finished_fifo;
static int device_open;

/** User Defined Data Item Structure */
struct data_item {
	unsigned int qid;
	unsigned long long time;
	char *msg;
};

/** Parameters passed to Module */
static int fifo_size;

/** Custom Function prototype */
int queueAlloc(int mem_size);

/**
	Function Name : fifo_module_read
	Function Type : Kernel Callback Method
	Description   : Method is invoked whenever the fifo device files are
			read. This callback method is triggered when a read 
			operation performed on the devices register to this 
			file operation object.
			FIFO Devices are allocated with one device being 
			read only(FIFO1) and other being write only(FIFO0).				
*/
static ssize_t fifo_module_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
	int ret;
	
		
	/** 
		Condition to check if the FIFO Queue is empty or in 
		underflow state
	*/
	if(!strlen(queue)) {
		printk(KERN_ALERT "FIFO ERROR:Fifo module cannot be read -> Underflow state.\n");	
		
			/** Erroneous Data */
		return -ENODATA;
	}
		
	printk(KERN_INFO "FIFO:Fifo module is being read.\n");	
		
	/** Condition to check if EOF is reached. */
	if(!finished_fifo) {
		ret = sprintf(buf,queue);
		if(ret <0) {
					/** Memory allocation problem */
			return -ENOMEM;
		}
		/** Flag set to Completed marking EOF.*/
		finished_fifo = 1;
		/** Successful execution of read callback with some bytes*/
		return ret;
	}
	
	/** FIFO Queue reloaded. */
	queue[0] = END_OF_BUFF;
	
	/** Successful execution of read callback with EOF reached.*/
	return 0;
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
	
	/**
			 Condition to check if the allocation is exceeded. To check
			 Overflow state is achieved.	
	*/
	if((strlen(buf)+strlen(queue))<=mem_alloc_size) {
		
		ret = sprintf((queue+strlen(queue)),buf);
		if(ret<0) {
			/** Memory allocation problem */
			return -ENOMEM;
		}
		printk(KERN_INFO "FIFO:Fifo module is being written.\n");
	}
	else {
		/** Overflow state block */
		printk(KERN_ALERT "FIFO ERROR:Fifo module in overflow state.\n");
		
		/** Buffer overflow problem */
		return -ENOBUFS;
	}

	/** Successful execution of write callback with buffer count.*/
	return count;
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
	/** Condition to check if the device is already in use. */
	if(device_open) {
		/** Device Busy Error */
		return -EBUSY;
	}
    
	/** 
	    Increment and using the device_open variable as a 
	    synchronization mechanism.
	*/
	device_open++;
	printk(KERN_INFO "FIFO ERROR:Fifo %d module is being opened.\n",iminor(inode));
	
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
	/** 
	    Decrement and using the device_open variable as a 
	    synchronization mechanism.
	*/
	device_open--;
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
		
		ret = sprintf(buf,"Allocated Size: %d\nFree Size: %d\nTotal Size: %d\n",strlen(queue),mem_alloc_size-strlen(queue),mem_alloc_size);
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
	long int res;
	int ret;
	printk(KERN_INFO "FIFO:Fifo config module is being written.\n");
	
	/** Condition to check if queue in use or not.*/
	if(strlen(queue)) {		
		printk(KERN_ALERT "FIFO ERROR:Fifo config module cannot be written.\n");
		
		/** DEVICE BUSY ERROR */
		return -EBUSY;
	}	
	
	/** Converting the string value to a number*/
	ret = kstrtol(buf,BASE_10,&res);
	if(ret < 0) {
		/** Invalid argument in conversion error.*/
		return -EINVAL;
	}
	
	/** Condition to check if the allocation is with in the limits.*/
	if(IN_RANGE(res)) {
		/** 
		    Condition to check if the queue allocation encountered any
		    problems.
		*/
		if(queueAlloc(res)!=0) {
			/** Memory allocation problem */
			return -ENOMEM;
		}
	}
	else {		
		printk(KERN_ALERT "FIFO ERROR:Fifo Queue cannot be allocated.\n");
		/** Memory allocation limit problem */
		return -ENOMEM;
	}
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
    File Operations for handling the /proc/fifo_config file accesses.
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
	
	/**Proc FS is created with RD&WR permissions with name fifo_config*/
	fifo_config_file_entry = proc_create(FIFO_CONFIG,0777,NULL,&fifo_config_module_fops);
	
	/** Condition to verify if fifo_config creation was successful*/
	if(fifo_config_file_entry == NULL) {
		printk(KERN_ALERT "FIFO ERROR: Could not initialize /proc/%s\n",FIFO_CONFIG);
		/** FILE CREATION PROBLEM */
		return -ENOMEM;
	}
	
	/** 
	    Registering the Device with a major number as 240 and
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
	
	/** Device Status flag set to false because device not in use.*/
   	device_open = 0;	
	/** Default Memory size of queue set to 8*/
	mem_alloc_size = DEFAULT_MEM_SIZE;

	/** Queue Allocated with the default size.*/
	queue = kmalloc(mem_alloc_size,GFP_KERNEL);
	
	/** Condition to check if the memory allocation was successful.*/
	if(!queue) {
		printk(KERN_ERR "FIFO ERROR:Memory allocation problem.\n");
		/** Memory allocation problem.*/
		return -ENOMEM;
	}
	
	/** FIFO HEAD Set to FIRST Location. */
	queue[0] = END_OF_BUFF;
	
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
	queue = kmalloc(mem_size,GFP_KERNEL);
	/** Condition to see if the allocation was successful or not.*/
	if(!queue) {
		printk(KERN_ERR "FIFO ERROR:Memory allocation problem.\n");
		
		/** Memory allocation problem.*/
		return -ENOMEM;
	}
	/** Setting the memory allocation size */
	mem_alloc_size = mem_size;
	
	/** FIFO HEAD Set to FIRST Location. */
	queue[0] = END_OF_BUFF;
	
	/** Successful execution of Queue Allocation method*/
	return 0;
}
/** Initializing the kernel module init with custom init method */
module_init(fifo_module_init);
/** Initializing the kernel module exit with custom cleanup method */
module_exit(fifo_module_cleanup);
/** Macro which deals with setting up of parameter passing to the module */
module_param(fifo_size,int,0);
