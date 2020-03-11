/* ----------------------------------------------- DRIVER hcsr --------------------------------------------------

 Basic driver example to show skelton methods for several file operations.

 ----------------------------------------------------------------------------------------------------------------*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/string.h>
#include <linux/device.h>
#include <linux/jiffies.h>

#include<linux/init.h>
#include<linux/moduleparam.h>

#include <linux/spinlock.h>
#include <linux/string.h>

#include <linux/proc_fs.h>
//#include <asm/uaccess.h>
#include <linux/uaccess.h>
#include <linux/interrupt.h>
#include <linux/miscdevice.h>

#define DEVICE_NAME_1                 "HCSR_1"  // device name to be created and registered
#define DEVICE_NAME_2                 "HCSR_2"

/* per device structure */
struct hcsr_dev {
	struct miscdevice misc;
	char name[20];                  /* Name of device*/
	char in_string[256];			/* buffer for the input string */
	spinlock_t spinlock;
};


struct hcsr_dev* HCSR_DEVPS;

static dev_t HCSR_DEV_NUMBER;      /* Allotted device number */
struct class* HCSR_DEV_CLASS;          /* Tie with the device model */
static struct device* HCSR_DEV_DEVICES;


static char* user_name = "Kamal";  /* the default user name, can be replaced if a new name is attached in insmod command */
// The number of devices to allocate
static int DEVICES = 2;



module_param(user_name,charp,0000);	//to get parameter from load.sh script to greet the user
//module_param(DEVICES, int, 0660);



/*
* Open hcsr driver
*/
int hcsr_driver_open(struct inode *inode, struct file *file)
{
	struct hcsr_dev *hcsr_devps;

	/* Get the per-device structure that contains this cdev */
	//hcsr_devps = container_of(inode->i_cdev, struct hcsr_dev, cdev);

	// Get the minor number
	dev_t minor_number = iminor(inode);

	// Search for matching device
	


	/* Easy access to cmos_devp from rest of the entry points */
	file->private_data = hcsr_devps;
	printk("\n%s is openning \n", hcsr_devps->name);
	return 0;
}


/*
 * Release hcsr driver
 */
int hcsr_driver_release(struct inode *inode, struct file *file)
{
	struct hcsr_dev *HCSR_DEVPS = file->private_data;

	printk("\n%s is closing\n", HCSR_DEVPS->name);

	return 0;
}

/*
 * Write to hcsr driver
 */
ssize_t hcsr_driver_write(struct file *file, const char *buf,
           size_t count, loff_t *ppos)
{
	struct hcsr_dev *HCSR_DEVPS = file->private_data;


	return 0;
}
/*
 * Read to hcsr driver
 */
ssize_t hcsr_driver_read(struct file *file, char *buf,
           size_t count, loff_t *ppos)
{
	int bytes_read = 0;
	struct hcsr_dev *HCSR_DEVPS = file->private_data;
	/*
	 * If we're at the end of the message,
	 * return 0 signifying end of file
	 */
	
	/*
	 * Most read functions return the number of bytes put into the buffer
	 */
	return bytes_read;

}



/* File operations structure. Defined in linux/fs.h */
static struct file_operations hcsr_fops = {
    .owner		= THIS_MODULE,           /* Owner */
    .open		= hcsr_driver_open,        /* Open method */
    .release	= hcsr_driver_release,     /* Release method */
    .write		= hcsr_driver_write,       /* Write method */
    .read		= hcsr_driver_read,        /* Read method */
};


/*
 * Driver Initialization
 */
int __init hcsr_driver_init(void)
{
	int ret;
	int time_since_boot;

	HCSR_DEVPS = kmalloc(sizeof(struct hcsr_dev), GFP_KERNEL);
	if (!HCSR_DEVPS) {
		printk("Bad Kmalloc\n"); return -ENOMEM;
	}
	memset(HCSR_DEVPS, 0, sizeof(struct hcsr_dev));

	// Initialize fields
	HCSR_DEVPS->misc.minor = 0;
	HCSR_DEVPS->misc.fops = &hcsr_fops;
	spin_lock_init(&(HCSR_DEVPS->spinlock));

	int verify = -1;

	verify = misc_register(&(HCSR_DEVPS->misc));
	if(verify != 0) {
		printk("did not register\n");
		memset(HCSR_DEVPS, 0, sizeof(struct hcsr_dev));
		kfree(HCSR_DEVPS);
		return -1;
	}
	


	
	return 0;
}


/* Driver Exit */
void __exit hcsr_driver_exit(void)
{
	

	misc_deregister(&(HCSR_DEVPS->misc));
	memset(HCSR_DEVPS, 0, sizeof(struct hcsr_dev));
	kfree(HCSR_DEVPS);

	printk("hcsr driver removed.\n");
}



module_init(hcsr_driver_init);
module_exit(hcsr_driver_exit);
MODULE_LICENSE("GPL v2");
