/**
	\file	:	task1_2.c
	\author	: 	Team Mango
	\brief	:	Task 1.2 of OS Lab-2 related to Clock driver implementation.
			Task involves use of a config file which modifies the clock
			info such as clock format.
*/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/time.h>
#include <linux/proc_fs.h>
#include <linux/kernel.h>

MODULE_AUTHOR("Team Mango");
MODULE_DESCRIPTION("Lab Solution Task1.2");
MODULE_LICENSE("GPL");

/** PROC FS RELATED MACROS */
#define PROC_FILE_NAME		"deeds_clock"
#define PROC_CONFIG_FILE_NAME	"deeds_clock_config"

/** STANDARD MACROS */
#define BASE_10 		10

/** TIME ZONE MACROS */
#define CEST			3600

/** Proc FS Dir Object */
static struct proc_dir_entry *proc_file_entry,*proc_config_file_entry;

/** Flags */
static int finished_clock;
static int finished_clock_config;

/** Clock Format Option */
static int option;

/**
	Function Name : deeds_clock_module_read
	Function Type : Kernel Callback Method
	Description   : Method is invoked whenever the deeds_clock file is
			read. This callback method is triggered when a read 
			operation performed on the above mentioned file
			which is registered to the file operation object.
			/proc/deeds_clock is a read only file. Whereas
			/proc/deeds_clock_config is a read/write file.
			Based on the value stored in the config file the
			clock format is modified. The possible values stored
			are 0,1.
*/
static ssize_t deeds_clock_module_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
	/** Timeval object defined for obtaining the current time in seconds. */
	struct timeval timeval_obj;
	/** Time object defined to vary the output based on specific time formats.*/
	struct tm tm_obj;
	
	int ret=0;
	
	/** 
		Kernel call obtains the current time and stores the information
		in timeval object.
	*/
	do_gettimeofday(&timeval_obj);
	
	printk(KERN_INFO "Deeds Clock Module read.\n");
	
	/** To check EOF is reached. */
	if(!finished_clock) {
	
		/** To check which clock format is in use. */
		if(option) {			
			/** Since option=1, clock format is yy-mm-dd h:m:s*/
			time_to_tm(get_seconds(),CEST/*sys_tz.tz_minuteswest * 60*/, &tm_obj);		
			ret = sprintf(buf,"current time:%04ld-%02d-%02d %02d:%02d:%02d\n", tm_obj.tm_year + 1900, tm_obj.tm_mon + 1, tm_obj.tm_mday, tm_obj.tm_hour, tm_obj.tm_min, tm_obj.tm_sec);
			if(ret < 0) {
				/** Memory Allocation Problem */
				return -ENOMEM;
			}
		}
		else {
			/** option = 0, clock format is in seconds.*/
			ret = sprintf(buf,"current time:%ld seconds\n",timeval_obj.tv_sec);
			if(ret <0) {
				/** Memory Allocation Problem */
				return -ENOMEM;			
			}
		}
		/** Flag set to true indicating EOF */
		finished_clock = 1;
		/** Total bytes read successfully returned.*/
		return ret;
	}	
	/** Successful execution of read call back. EOF reached.*/
	return 0;
}

/**
	Function Name : deeds_clock_config_module_read
	Function Type : Kernel Callback Method
	Description   : Method is invoked whenever the deeds_config_clock
			file is read. This callback method is triggered 
			when a read operation performed on the above 
			mentioned file which is registered to the file 
			operation object. 
			/proc/deeds_clock is a read only file. Whereas
			/proc/deeds_clock_config is a read/write file.
			Based on the value stored in the config file the
			clock format is modified. The possible values stored
			are 0,1.
*/
static ssize_t deeds_clock_config_module_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
	int ret;
	/** To check EOF is reached. */
	if(!finished_clock_config) {
		ret = sprintf(buf,"current clock format:%d \n",option);
		if(ret < 0) {
			/** Memory Allocation Problem */
			return -ENOMEM;		
		}
		/** Flag set to true indicating EOF */
		finished_clock_config = 1;
		/** Total bytes read successfully returned.*/
		return ret;
	}
	/** Successful execution of read call back. EOF reached.*/
	return 0;
}


/**
	Function Name : deeds_clock_config_module_write
	Function Type : Kernel Callback Method
	Description   : Method is invoked whenever the deeds_config_clock
			file is written. This callback method is triggered 
			when a write operation performed on the above 
			mentioned file which is registered to the file 
			operation object. 
			/proc/deeds_clock is a read only file. Whereas
			/proc/deeds_clock_config is a read/write file.
			Based on the value stored in the config file the
			clock format is modified. The possible values stored
			are 0,1.
*/
static ssize_t deeds_clock_config_module_write(struct file *file, const char *buf, size_t count, loff_t *ppos)
{
	int ret;
	long int input_clock_format;
	printk(KERN_INFO "Task1.2 Module written.\n");
	
	ret = kstrtol(buf,BASE_10,&input_clock_format);
	if(ret < 0) {
		/** Invalid argument in conversion error.*/
		return -EINVAL;
	}
	
	/** Check if the provided clock format option is 1 or not.*/
	if(input_clock_format==1) {
		printk(KERN_INFO "1 is written.\n");
		option = 1;
	}
	/** Check if the provided clock format option is 0 or not.*/
	else if(input_clock_format==0) {
		printk(KERN_INFO "0 is written.\n");
		option = 0;
	}
	else {
		printk(KERN_ALERT "Invalid character is written.\n");
		/** Invalid data ERROR*/
		return -EINVAL;		
	}
	/** 
		Successful execution of write callback. Returns the 
		number of bytes written.
	*/
	return count;
}

/**
	Function Name : deeds_clock_module_open
	Function Type : Kernel Callback Method
	Description   : Method is invoked whenever the deeds_clock
			file is opened. This callback method is triggered 
			when an open operation performed on the above 
			mentioned file which is registered to the file 
			operation object. 
			/proc/deeds_clock is a read only file. Whereas
			/proc/deeds_clock_config is a read/write file.
			Based on the value stored in the config file the
			clock format is modified. The possible values stored
			are 0,1.
*/
static int deeds_clock_module_open(struct inode * inode, struct file * file)
{
	printk(KERN_INFO "Deeds Clock Module opened.\n");
	/** Flag set to false marking file is not yet read */
	finished_clock=0;
	/** Successful execution of open callback method.*/
	return 0;
}

/**
	Function Name : deeds_clock_config_module_open
	Function Type : Kernel Callback Method
	Description   : Method is invoked whenever the deeds_clock_config
			file is opened. This callback method is triggered 
			when an open operation performed on the above 
			mentioned file which is registered to the file 
			operation object. 
			/proc/deeds_clock is a read only file. Whereas
			/proc/deeds_clock_config is a read/write file.
			Based on the value stored in the config file the
			clock format is modified. The possible values stored
			are 0,1.
*/static int deeds_clock_config_module_open(struct inode * inode, struct file * file)
{
	printk(KERN_INFO "Deeds Clock config Module opened.\n");
	/** Flag set to false marking file is not yet read */
	finished_clock_config=0;
	/** Successful execution of open callback method.*/
	return 0;
}

/**
	Function Name : deeds_clock_module_release
	Function Type : Kernel Callback Method
	Description   : Method is invoked whenever the deeds_clock
			file is closed. This callback method is triggered 
			when an close operation performed on the above 
			mentioned file which is registered to the file 
			operation object. 
			/proc/deeds_clock is a read only file. Whereas
			/proc/deeds_clock_config is a read/write file.
			Based on the value stored in the config file the
			clock format is modified. The possible values stored
			are 0,1.
*/
static int deeds_clock_module_release(struct inode * inode, struct file * file)
{
	printk(KERN_INFO "Deeds clock Module released.\n");
	/** Successful execution of release callback.*/
	return 0;
}


/**
	Function Name : deeds_clock_config_module_release
	Function Type : Kernel Callback Method
	Description   : Method is invoked whenever the deeds_clock_config
			file is closed. This callback method is triggered 
			when an close operation performed on the above 
			mentioned file which is registered to the file 
			operation object. 
			/proc/deeds_clock is a read only file. Whereas
			/proc/deeds_clock_config is a read/write file.
			Based on the value stored in the config file the
			clock format is modified. The possible values stored
			are 0,1.
*/
static int deeds_clock_config_module_release(struct inode * inode, struct file * file)
{
	printk(KERN_INFO "Deeds clock config Module released.\n");
	/** Successful execution of release callback.*/
	return 0;
}

/** File operations related to deeds_clock file */
static struct file_operations deeds_clock_module_fops = {
	.owner =	THIS_MODULE,
	.read =		deeds_clock_module_read,
	.open =		deeds_clock_module_open,
	.release =	deeds_clock_module_release,
};

/** File operations related to deeds_clock_config file */
static struct file_operations deeds_config_module_fops = {
	.owner =	THIS_MODULE,
	.read =		deeds_clock_config_module_read,
	.write =	deeds_clock_config_module_write,
	.open =		deeds_clock_config_module_open,
	.release =	deeds_clock_config_module_release,
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
	printk(KERN_INFO "Task1.2 module is being loaded.\n");

	/**Proc FS is created with RDONLY permissions with name deeds_clock*/
	proc_file_entry = proc_create(PROC_FILE_NAME,0644,NULL,&deeds_clock_module_fops);		
	/** Condition to verify if deeds_clock creation was successful*/
	if(proc_file_entry == NULL) {
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",PROC_FILE_NAME);
		/** File Creation problem.*/
		return -ENOMEM;
	}
	
	/**Proc FS is created with RD&WR permissions with name deeds_clock_config*/
	proc_config_file_entry = proc_create(PROC_CONFIG_FILE_NAME,0777,NULL,&deeds_config_module_fops);
	/** Condition to verify if deeds_clock_config creation was successful*/
	if(proc_config_file_entry == NULL) {
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",PROC_CONFIG_FILE_NAME);
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
	printk(KERN_INFO "Task1.2 module is being unloaded.\n");
	/** Proc FS object related to /deeds_clock removed.*/
	proc_remove(proc_file_entry);
	/** Proc FS object related to /deeds_clock_config removed.*/
	proc_remove(proc_config_file_entry);
}
/** Initializing the kernel module init with custom init method */
module_init(gen_module_init);
/** Initializing the kernel module exit with custom cleanup method */
module_exit(gen_module_cleanup);
