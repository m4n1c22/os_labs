#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/time.h>
#include <linux/proc_fs.h>

MODULE_AUTHOR("Sreeram Sadasivam");
MODULE_DESCRIPTION("Lab Solution Task1.1");
MODULE_LICENSE("GPL");

//Macros

#define PROC_FILE_NAME	"deeds_clock"

//procfs_dir object

static struct proc_dir_entry *proc_file_entry;

// this method is executed when reading from the module
static ssize_t gen_module_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
	struct timeval timeval_obj;
	do_gettimeofday(&timeval_obj);
	printk(KERN_INFO "Task1.1 Module read.\n");
	sprintf(buf,"current time:%ld seconds",timeval_obj.tv_sec);
	printk(KERN_INFO "current time:%ld seconds \n",timeval_obj.tv_sec);
	return 0;
}
// this method is called whenever the module is being used
// e.g. for both read and write operations
static int gen_module_open(struct inode * inode, struct file * file)
{
	printk(KERN_INFO "Task1.1 Module opened.\n");
	return 0;
}

// this method releases the module and makes it available for new operations
static int gen_module_release(struct inode * inode, struct file * file)
{
	printk(KERN_INFO "Task1.1 Module released.\n");
	return 0;
}

// module's file operations, a module may need more of these
static struct file_operations gen_module_fops = {
	.owner =	THIS_MODULE,
	.read =		gen_module_read,
	.open =		gen_module_open,
	.release =	gen_module_release,
};

// initialize module (executed when using insmod)
static int __init gen_module_init(void)
{
	printk(KERN_INFO "Task1.1 module is being loaded.\n");

	proc_file_entry = proc_create(PROC_FILE_NAME,0,NULL,&gen_module_fops);	
	
	if(proc_file_entry == NULL) {
		proc_remove(proc_file_entry);
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",PROC_FILE_NAME);
		return -ENOMEM;
	}
	
	return 0;
}

// cleanup module (executed when using rmmod)
static void __exit gen_module_cleanup(void)
{
	printk(KERN_INFO "Task1.1 module is being unloaded.\n");
	proc_remove(proc_file_entry);
}

module_init(gen_module_init);
module_exit(gen_module_cleanup);
