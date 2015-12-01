/**
	\file	:	task1_1.c
	\author	: 	Team Mango
	\brief	:	Task 1.1 of OS Lab-2 related to Clock driver implementation.
			Task involves use of a clock file which prints the current
			time in seconds.
*/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/time.h>
#include <linux/proc_fs.h>

MODULE_AUTHOR("Team Mango");
MODULE_DESCRIPTION("Lab Solution Task1.1");
MODULE_LICENSE("GPL");

/** PROC FS RELATED  MACROS */
#define PROC_FILE_NAME	"deeds_clock"

/** Proc FS Dir Object */
static struct proc_dir_entry *proc_file_entry;
/** Flags */
static int finished;

/**
	Function Name : gen_module_read
	Function Type : Kernel Callback Method
	Description   : Method is invoked whenever the deeds_clock file is
			read. This callback method is triggered when a read 
			operation performed on the above mentioned file
			which is registered to the file operation object.
			/proc/deeds_clock is a read only file. 
*/
static ssize_t gen_module_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
	/** Timeval object defined for obtaining the current time in seconds. */
	struct timeval timeval_obj;
	int ret=0;
		
	/** 
		Kernel call obtains the current time and stores the information
		in timeval object.
	*/
	do_gettimeofday(&timeval_obj);
	
	printk(KERN_INFO "Task1.1 Module read.\n");
	
	/** To check EOF is reached. */
	if(!finished) {
		ret = sprintf(buf,"current time:%ld seconds\n",timeval_obj.tv_sec);
		if(ret < 0) {
			/** Memory Allocation Problem */
			return -ENOMEM;
		}
		/** Flag set to true indicating EOF */
		finished = 1;
		/** Total bytes read successfully returned.*/	
		return ret;
	}	
	/** Successful execution of read call back. EOF reached.*/
	return 0;
}

/**
	Function Name : gen_module_open
	Function Type : Kernel Callback Method
	Description   : Method is invoked whenever the deeds_clock
			file is opened. This callback method is triggered 
			when an open operation performed on the above 
			mentioned file which is registered to the file 
			operation object. 
*/
static int gen_module_open(struct inode * inode, struct file * file)
{
	/** Flag set to false marking file is not yet read */
	finished = 0;
	printk(KERN_INFO "Task1.1 Module opened.\n");
	/** Successful execution of open callback method.*/
	return 0;
}

/**
	Function Name : gen_module_release
	Function Type : Kernel Callback Method
	Description   : Method is invoked whenever the deeds_clock
			file is closed. This callback method is triggered 
			when an close operation performed on the above 
			mentioned file which is registered to the file 
			operation object. 
*/
static int gen_module_release(struct inode * inode, struct file * file)
{
	printk(KERN_INFO "Task1.1 Module released.\n");
	/** Successful execution of release callback.*/
	return 0;
}

/** File operations related to deeds_clock file */
static struct file_operations gen_module_fops = {
	.owner =	THIS_MODULE,
	.read =		gen_module_read,
	.open =		gen_module_open,
	.release =	gen_module_release,
};

/**
	Function Name : gen_module_init
	Function Type : Module INIT
	Description   : Initialization method of the Kernel module. The
			method gets invoked when the kernel module is being
			inserted using the command insmod.
*/
static int __init gen_module_init(void)
{
	printk(KERN_INFO "Task1.1 module is being loaded.\n");
	
	/**Proc FS is created with RDONLY permissions with name deeds_clock*/
	proc_file_entry = proc_create(PROC_FILE_NAME,0,NULL,&gen_module_fops);	
	/** Condition to verify if deeds_clock creation was successful*/
	if(proc_file_entry == NULL) {
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",PROC_FILE_NAME);
		/** File Creation problem.*/
		return -ENOMEM;
	}
	/** Successful execution of initialization method. */
	return 0;
}

/**
	Function Name : gen_module_cleanup
	Function Type : Module EXIT
	Description   : Cleanup method of the Kernel module. The
                	method gets invoked when the kernel module is being
                 	removed using the command rmmod.
*/
static void __exit gen_module_cleanup(void)
{
	printk(KERN_INFO "Task1.1 module is being unloaded.\n");
	/** Proc FS object related to /deeds_clock removed.*/	
	proc_remove(proc_file_entry);
}
/** Initializing the kernel module init with custom init method */
module_init(gen_module_init);
/** Initializing the kernel module exit with custom cleanup method */
module_exit(gen_module_cleanup);
