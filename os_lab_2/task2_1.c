#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/device.h> 
#include <asm/uaccess.h>
#include <linux/kernel.h>
#include <linux/slab.h>

MODULE_AUTHOR("Sreeram Sadasivam");
MODULE_DESCRIPTION("Lab Solution Task 2.1");
MODULE_LICENSE("GPL");

//Macros
#define FIFO0_DEVICE		"fifo0"
#define FIFO1_DEVICE		"fifo1"
#define FIFO_DEVICE			"fifo"
#define FIFO_CONFIG			"fifo_config"
#define MAJOR_NUM			240
#define MINOR_NUM_FIFO0		0
#define MINOR_NUM_FIFO1		1
#define CLASS_NAME  		"fifo_class"

#define IS_MINOR(A,B)	    (A==B)
static struct proc_dir_entry *fifo_config_file_entry;

static struct class*  fifoClass  = NULL; 
static struct device* fifo0 = NULL; 
static struct device* fifo1 = NULL;

static char *msg;

static int mem_alloc_size=10;

static int minorNumber;

static int finished_config,finished_fifo;
static int majorNumber;
// this method is executed when reading from the module
static ssize_t fifo_module_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
	int ret;
	printk(KERN_INFO "Fifo module is being read.\n");
	
	if(IS_MINOR(minorNumber,MINOR_NUM_FIFO0)) {
		
		printk(KERN_INFO "Fifo module is being read.\n");	
		if(!finished_fifo) {
			ret = sprintf(buf,msg);
			if(ret <0) {
				return -ENOMEM;
			}
			finished_fifo = 1;
			return ret;
		}
	}
	else {
		printk(KERN_INFO "Fifo module not allowed to be read.\n");
	}
	msg[0] = '\0';
	return 0;
}

// this method is executed when writing to the module
static ssize_t fifo_module_write(struct file *file, const char *buf, size_t count, loff_t *ppos)
{
	int ret;
	printk(KERN_INFO "Fifo module is being written.\n");
	
	if(IS_MINOR(minorNumber,MINOR_NUM_FIFO1)) {
		if(strlen(buf)<=mem_alloc_size) {
			ret = sprintf(msg,buf);
			if(ret<0) {
				return -ENOMEM;
			}
		}
		else {
			printk(KERN_ERR "Fifo module in overflow state.\n");
			return -ENOMEM;
		}
	}
	else {
		return -1;
	}
	
	return count;
}


// this method is called whenever the module is being used
// e.g. for both read and write operations
static int fifo_module_open(struct inode * inode, struct file * file)
{
	printk(KERN_INFO "Fifo module is being opened.\n");
	printk(KERN_INFO "Fifo %d module is being opened.\n",iminor(inode));
	minorNumber = iminor(inode);
	if(IS_MINOR(minorNumber,MINOR_NUM_FIFO0)) {
		finished_fifo = 0;
	}
	return 0;
}

// this method releases the module and makes it available for new operations
static int fifo_module_release(struct inode * inode, struct file * file)
{
	printk(KERN_INFO "Fifo module is being released.\n");
	return 0;
}



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
static struct file_operations fifo_module_fops = {
	.owner =	THIS_MODULE,
	.read =		fifo_module_read,
	.write =	fifo_module_write,
	.open =		fifo_module_open,
	.release =	fifo_module_release,
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
	
	// Register the character device (atleast try) 
	majorNumber = register_chrdev(MAJOR_NUM, FIFO_DEVICE, &fifo_module_fops);
	if (majorNumber < 0) {
		printk(KERN_ALERT "%s failed with %d\n",
		       "Sorry, registering the character device ", MAJOR_NUM);
		return majorNumber;
	}
	printk(KERN_INFO "FIFO: registered correctly with major number %d\n", MAJOR_NUM);

   // Register the device class
   fifoClass = class_create(THIS_MODULE, CLASS_NAME);
   if (IS_ERR(fifoClass)){                // Check for error and clean up if there is
      unregister_chrdev(MAJOR_NUM, FIFO_DEVICE);
      printk(KERN_ALERT "Failed to register device class\n");
      return PTR_ERR(fifoClass);          // Correct way to return an error on a pointer
   }
   printk(KERN_INFO "FIFO: device class registered correctly\n");

   // Register the device driver
   fifo0 = device_create(fifoClass, NULL, MKDEV(MAJOR_NUM, MINOR_NUM_FIFO0), NULL, FIFO0_DEVICE);
   if (IS_ERR(fifo0)){               // Clean up if there is an error
      class_destroy(fifoClass);           // Repeated code but the alternative is goto statements
      unregister_chrdev(MAJOR_NUM, FIFO_DEVICE);
      printk(KERN_ALERT "Failed to create the device\n");
      return PTR_ERR(fifo0);
   }
   printk(KERN_INFO "FIFO: device class created correctly\n"); // Made it! device was initialized

   fifo1 = device_create(fifoClass, NULL, MKDEV(MAJOR_NUM, MINOR_NUM_FIFO1), NULL, FIFO1_DEVICE);
   if (IS_ERR(fifo1)){               // Clean up if there is an error
      class_destroy(fifoClass);           // Repeated code but the alternative is goto statements
      unregister_chrdev(MAJOR_NUM, FIFO_DEVICE);
      printk(KERN_ALERT "Failed to create the device\n");
      return PTR_ERR(fifo1);
   }
   printk(KERN_INFO "FIFO: device class created correctly\n"); // Made it! device was initialized


   msg = kmalloc(mem_alloc_size,GFP_KERNEL);
   if(!msg) {
		printk(KERN_ERR "Memory allocation problem.\n");
		return -ENOMEM;
   }
   msg[0] = '\0';
	
   return 0;
}

// cleanup module (executed when using rmmod)
static void __exit fifo_module_cleanup(void)
{
	
	printk(KERN_INFO "FIFO module is being unloaded.\n");
	proc_remove(fifo_config_file_entry);
	
	// remove the device
	device_destroy(fifoClass, MKDEV(MAJOR_NUM, MINOR_NUM_FIFO0));
	// remove the device
	device_destroy(fifoClass, MKDEV(MAJOR_NUM, MINOR_NUM_FIFO1));     
	// unregister the device class
	class_unregister(fifoClass);                          
	// remove the device class
	class_destroy(fifoClass);                             
	//Unregister the device 
	unregister_chrdev(MAJOR_NUM, FIFO_DEVICE);
	
	//deallocating queue
	kfree(msg);

}

module_init(fifo_module_init);
module_exit(fifo_module_cleanup);
