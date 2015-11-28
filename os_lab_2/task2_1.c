#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/time.h>
#include <linux/proc_fs.h>

MODULE_AUTHOR("Sreeram Sadasivam");
MODULE_DESCRIPTION("Lab Solution Task 2.1");
MODULE_LICENSE("GPL");

//Macros
#define FIFO_RDONLY_DEVICE		"/dev/fifo0"
#define FIFO_WTONLY_DEVICE		"/dev/fifo1"
#define FIFO_CONFIG				"fifo_config"

static struct proc_dir_entry *fifo_config_file_entry;

static int finished_config;

// this method is executed when reading from the module
static ssize_t fifo_config_module_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
	int ret;
	printk(KERN_INFO "Fifo config module is being read.\n");
	if(!finished_config){
		finished_config = 1;
		return ret;
	}
	return 0;
}

// this method is executed when writing to the module
static ssize_t fifo_config_module_write(struct file *file, const char *buf, size_t count, loff_t *ppos)
{
	printk(KERN_INFO "Fifo config module is being written.\n");
	return count;
}

// this method is called whenever the module is being used
// e.g. for both read and write operations
static int fifo_config_module_open(struct inode * inode, struct file * file)
{
	printk(KERN_INFO "Fifo config module is being opened.\n");
	finished_config = 0;
	return 0;
}

// this method releases the module and makes it available for new operations
static int fifo_config_module_release(struct inode * inode, struct file * file)
{
	printk(KERN_INFO "Fifo config module is being released.\n");
	return 0;
}

// module's file operations, a module may need more of these
static struct file_operations fifo_config_module_fops = {
	.owner =	THIS_MODULE,
	.read =		fifo_config_module_read,
	.write =	fifo_config_module_write,
	.open =		fifo_config_module_open,
	.release =	fifo_config_module_release,
};

// initialize module (executed when using insmod)
static int __init fifo_module_init(void)
{	
	printk(KERN_INFO "FIFO module is being loaded.\n");	
	fifo_config_file_entry = proc_create(FIFO_CONFIG,0777,NULL,&fifo_config_module_fops);
	
	if(fifo_config_file_entry == NULL) {
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",FIFO_CONFIG);
		return -ENOMEM;
	}
	return 0;
}

// cleanup module (executed when using rmmod)
static void __exit fifo_module_cleanup(void)
{
	printk(KERN_INFO "FIFO module is being unloaded.\n");
	proc_remove(fifo_config_file_entry);
}

module_init(fifo_module_init);
module_exit(fifo_module_cleanup);
