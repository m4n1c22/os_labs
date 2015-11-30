#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/time.h>
#include <linux/proc_fs.h>

MODULE_AUTHOR("Sreeram Sadasivam");
MODULE_DESCRIPTION("Lab Solution Task1.2");
MODULE_LICENSE("GPL");

//Macros

#define PROC_FILE_NAME			"deeds_clock"
#define PROC_CONFIG_FILE_NAME	"deeds_clock_config"

//procfs_dir object

static struct proc_dir_entry *proc_file_entry,*proc_config_file_entry;

static int finished_clock,finished_clock_config;
static int option;

// this method is executed when reading from the module
static ssize_t deeds_clock_module_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
	struct timeval timeval_obj;
	struct tm tm_obj;
	int ret=0;
	
	do_gettimeofday(&timeval_obj);
	
	printk(KERN_INFO "Deeds Clock Module read.\n");
	if(!finished_clock) {
		if(option) {			
			time_to_tm(get_seconds(),sys_tz.tz_minuteswest * 60, &tm_obj);		
			ret = sprintf(buf,"current time:%04ld-%02d-%02d %02d:%02d:%02d\n", tm_obj.tm_year + 1900, tm_obj.tm_mon + 1, tm_obj.tm_mday, tm_obj.tm_hour, tm_obj.tm_min, tm_obj.tm_sec);
		}
		else {			
			ret = sprintf(buf,"current time:%ld seconds\n",timeval_obj.tv_sec);
			if(ret <0) {
				return -ENOMEM;			
			}
		}
		finished_clock = 1;
		return ret;
	}	
	return 0;
}

// this method is executed when reading from the module
static ssize_t deeds_clock_config_module_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
	int ret;
	if(!finished_clock_config) {
		ret = sprintf(buf,"current clock format:%d \n",option);
		if(ret < 0) {
			return -ENOMEM;		
		}
		finished_clock_config = 1;
		return ret;
	}	
	return 0;
}


// this method is executed when writing to the module
static ssize_t deeds_clock_config_module_write(struct file *file, const char *buf, size_t count, loff_t *ppos)
{
	printk(KERN_INFO "Task1.2 Module written.\n");
	
	if(strncmp(buf,"1",1)==0) {
		printk(KERN_INFO "1 is written.\n");
		option = 1;
	}
	else if(strncmp(buf,"0",1)==0) {
		printk(KERN_INFO "0 is written.\n");
		option = 0;
	}
	else {
		printk(KERN_ALERT "Invalid character is written.\n");
		return -EINVAL;		
	}
	return count;
}

// this method is called whenever the module is being used
// e.g. for both read and write operations
static int deeds_clock_module_open(struct inode * inode, struct file * file)
{
	printk(KERN_INFO "Deeds Clock Module opened.\n");
	finished_clock=0;
	return 0;
}

// this method is called whenever the module is being used
// e.g. for both read and write operations
static int deeds_clock_config_module_open(struct inode * inode, struct file * file)
{
	printk(KERN_INFO "Deeds Clock config Module opened.\n");
	finished_clock_config=0;
	return 0;
}

// this method releases the module and makes it available for new operations
static int deeds_clock_module_release(struct inode * inode, struct file * file)
{
	printk(KERN_INFO "Deeds clock Module released.\n");
	return 0;
}


// this method releases the module and makes it available for new operations
static int deeds_clock_config_module_release(struct inode * inode, struct file * file)
{
	printk(KERN_INFO "Deeds clock config Module released.\n");
	return 0;
}

// module's file operations, a module may need more of these
static struct file_operations deeds_clock_module_fops = {
	.owner =	THIS_MODULE,
	.read =		deeds_clock_module_read,
	.open =		deeds_clock_module_open,
	.release =	deeds_clock_module_release,
};

static struct file_operations deeds_config_module_fops = {
	.owner =	THIS_MODULE,
	.read =		deeds_clock_config_module_read,
	.write =	deeds_clock_config_module_write,
	.open =		deeds_clock_config_module_open,
	.release =	deeds_clock_config_module_release,
};

// initialize module (executed when using insmod)
static int __init gen_module_init(void)
{
	printk(KERN_INFO "Task1.2 module is being loaded.\n");

	proc_file_entry = proc_create(PROC_FILE_NAME,0644,NULL,&deeds_clock_module_fops);		
	
	if(proc_file_entry == NULL) {
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",PROC_FILE_NAME);
		return -ENOMEM;
	}
	
	proc_config_file_entry = proc_create(PROC_CONFIG_FILE_NAME,0777,NULL,&deeds_config_module_fops);
	
	
	if(proc_config_file_entry == NULL) {
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",PROC_CONFIG_FILE_NAME);
		return -ENOMEM;
	}
	return 0;
}

// cleanup module (executed when using rmmod)
static void __exit gen_module_cleanup(void)
{
	printk(KERN_INFO "Task1.2 module is being unloaded.\n");
	proc_remove(proc_file_entry);
	proc_remove(proc_config_file_entry);
}

module_init(gen_module_init);
module_exit(gen_module_cleanup);
