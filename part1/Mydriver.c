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

#define DEVICE_NAME                 "hcsr"  // device name to be created and registered

/* per device structure */
struct hcsr_dev {
	struct cdev cdev;               /* The cdev structure */
	char name[20];                  /* Name of device*/
	char in_string[256];			/* buffer for the input string */
	int current_write_pointer;
	int minor_number;
	spinlock_t spinlockDevice;

};


struct hcsr_dev* HCSR_DEVPS;

static dev_t HCSR_DEV_NUMBER;      /* Allotted device number */
struct class *HCSR_DEV_CLASS;          /* Tie with the device model */
static struct device *HCSR_DEV_DEVICES;


static char *user_name = "Dear John";  /* the default user name, can be replaced if a new name is attached in insmod command */
// The number of devices to allocate
static int DEVICES = 0;



module_param(user_name,charp,0000);	//to get parameter from load.sh script to greet the user
module_param(DEVICES, int, 0000);


/*
* Open hcsr driver
*/
int hcsr_driver_open(struct inode *inode, struct file *file)
{
	struct hcsr_dev *HCSR_DEVPS;

	/* Get the per-device structure that contains this cdev */
	HCSR_DEVPS = container_of(inode->i_cdev, struct hcsr_dev, cdev);


	/* Easy access to cmos_devp from rest of the entry points */
	file->private_data = HCSR_DEVPS;
	printk("\n%s is openning \n", HCSR_DEVPS->name);
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

	while (count) {
		get_user(HCSR_DEVPS->in_string[HCSR_DEVPS->current_write_pointer], buf++);
		count--;
		if(count){
			HCSR_DEVPS->current_write_pointer++;
			if( HCSR_DEVPS->current_write_pointer == 256)
				HCSR_DEVPS->current_write_pointer = 0;
		}
	}
	printk("Writing -- %d %s \n", HCSR_DEVPS->current_write_pointer, HCSR_DEVPS->in_string);
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
	if (HCSR_DEVPS->in_string[0] == 0)
		return 0;

	/*
	 * Actually put the data into the buffer
	 */
	while (count && HCSR_DEVPS->in_string[bytes_read]) {

		put_user(HCSR_DEVPS->in_string[bytes_read], buf++);
		count--;
		bytes_read++;
	}
	printk("Reading -- '%s'\n",HCSR_DEVPS->in_string);
	/*
	 * Most read functions return the number of bytes put into the buffer
	 */
	return bytes_read;

}


long hcsr_driver_unlocked_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_param) {
	return 0;
}

/* File operations structure. Defined in linux/fs.h */
static struct file_operations hcsr_fops = {
    .owner		= THIS_MODULE,           /* Owner */
    .open		= hcsr_driver_open,        /* Open method */
    .release	= hcsr_driver_release,     /* Release method */
    .write		= hcsr_driver_write,       /* Write method */
    .read		= hcsr_driver_read,        /* Read method */
    .unlocked_ioctl      = hcsr_driver_unlocked_ioctl
};


/*
 * Driver Initialization
 */
int __init hcsr_driver_init(void)
{
	int ret;
	int time_since_boot;

	/* Request dynamic allocation of a device major number */
	if (alloc_chrdev_region(&HCSR_DEV_NUMBER, 0, DEVICES, DEVICE_NAME) < 0) {
			printk(KERN_DEBUG "Can't register device\n"); 
			return -1;
	}

	/* Populate sysfs entries */
	HCSR_DEV_CLASS = class_create(THIS_MODULE, DEVICE_NAME);
	if(IS_ERR(&HCSR_DEV_CLASS)) {
		// TODO: Add unregister_chrdev_region
		printk("class_create(...) ERROR\n");
		// Release the major number /
	    unregister_chrdev_region((HCSR_DEV_NUMBER), 1);
		return(PTR_ERR(HCSR_DEV_CLASS));
	}

	/* Allocate memory for the per-device structure */
	HCSR_DEVPS = kmalloc(DEVICES * sizeof(struct hcsr_dev), GFP_KERNEL);

	if (!HCSR_DEVPS) {
		printk("Bad Kmalloc\n"); 
		return -ENOMEM;
	}
	// Zero Memory
	memset(HCSR_DEVPS, 0, DEVICES * sizeof(struct hcsr_dev));


	/* Request I/O region */
	sprintf(HCSR_DEVPS->name, DEVICE_NAME);


	// Allocate array of device pointers (There should be DEVICE number of pointers)
	HCSR_DEV_DEVICES = kmalloc(DEVICES * sizeof(struct hcsr_dev*), GFP_KERNEL);
	if (!HCSR_DEV_DEVICES) {
		printk("Bad Kmalloc\n");
		// Free earlier array
		memset(HCSR_DEVPS, 0, DEVICES * sizeof(struct hcsr_dev));
		kfree(HCSR_DEVPS);
		return -ENOMEM;
	}

	// Zero memory
	memset(HCSR_DEV_DEVICES, 0, DEVICES * sizeof(struct hcsr_dev*));


	// Initialize All Devices
	char device_name = {'H', 'C', 'S', 'R', '_', '0', '\0'};
	int minor_num = 0;
	for(; minor_num < DEVICES; minor_num++) {

		/* Connect the file operations with the cdev */
		cdev_init(&HCSR_DEVPS[minor_num].cdev, &hcsr_fops);
		HCSR_DEVPS[minor_num].cdev.owner = THIS_MODULE;

		// TODO: initialize fields
		spin_lock_init(&(HCSR_DEVPS[minor_num].spinlockDevice));
		HCSR_DEVPS[minor_num].minor_number = minor_num;

		/* Connect the major/minor number to the cdev */
		ret = cdev_add(&HCSR_DEVPS[minor_num].cdev, MKDEV(MAJOR(HCSR_DEV_NUMBER), minor_num), 1);
		if (ret) {
			printk("Bad cdev\n");
			// Free earlier arrays
			memset(HCSR_DEVPS, 0, DEVICES * sizeof(struct hcsr_dev));
			kfree(HCSR_DEVPS);
			memset(HCSR_DEV_DEVICES, 0, DEVICES * sizeof(struct hcsr_dev*));
			kfree(HCSR_DEV_DEVICES);
			return ret;
		}

		// Name device
		device_name[5] = (minor_num + 1) + '0';

		// Create the device -> Send uevents to udev, so it'll create /dev nodes 
		HCSR_DEV_DEVICES[minor_num] = device_create(HCSR_DEV_CLASS, NULL, MKDEV(MAJOR(HCSR_DEV_NUMBER), minor_num), NULL, device_name);
		
		// Error Check
		if(IS_ERR(&HCSR_DEV_DEVICES[minor_num])) {

		class_destroy(HCSR_DEV_CLASS);
	    unregister_chrdev_region((HCSR_DEV_NUMBER), DEVICES);
	    // Free earlier arrays
		memset(HCSR_DEVPS, 0, DEVICES * sizeof(struct hcsr_dev));
		kfree(HCSR_DEVPS);
		memset(HCSR_DEV_DEVICES, 0, DEVICES * sizeof(struct hcsr_dev*));
		kfree(HCSR_DEV_DEVICES);
		printk("device_create(...) ERROR For minor_number = %d\n", minor_num);
		return PTR_ERR(HCSR_DEV_DEVICES);
		}

	}

	time_since_boot=(jiffies-INITIAL_JIFFIES)/HZ;//since on some systems jiffies is a very huge uninitialized value at boot and saved.
	sprintf(HCSR_DEVPS->in_string, "Hi %s, this machine has been on for %d seconds", user_name, time_since_boot);

	HCSR_DEVPS->current_write_pointer = 0;

	printk("hcsr driver initialized.\n'%s'\n",HCSR_DEVPS->in_string);
	return 0;
}


/* Driver Exit */
void __exit hcsr_driver_exit(void)
{
	// device_remove_file(HCSR_DEV_DEVICES, &dev_attr_xxx);
	/* Release the major number */
	unregister_chrdev_region((HCSR_DEV_NUMBER), 1);

	/* Destroy devices */
	int minor_num = 0;
	for(; minor_num < DEVICES; minor_num++) {
		device_destroy (HCSR_DEV_CLASS, MKDEV(MAJOR(HCSR_DEV_NUMBER), minor_num));
		cdev_del(&HCSR_DEVPS[minor_num].cdev);
	}

	// Zero out and free memory
	// Free earlier arrays
	memset(HCSR_DEVPS, 0, DEVICES * sizeof(struct hcsr_dev));
	kfree(HCSR_DEVPS);
	memset(HCSR_DEV_DEVICES, 0, DEVICES * sizeof(struct hcsr_dev*));
	kfree(HCSR_DEV_DEVICES);



	/* Destroy driver_class */
	class_destroy(HCSR_DEV_CLASS);

	printk("hcsr driver removed.\n");
}

module_init(hcsr_driver_init);
module_exit(hcsr_driver_exit);
MODULE_LICENSE("GPL v2");
