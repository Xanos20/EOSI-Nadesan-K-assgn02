/* ----------------------------------------------- DRIVER rbtree --------------------------------------------------
 
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

#include <linux/rbtree.h>
#include <linux/spinlock.h>


#include <linux/proc_fs.h>
//#include <asm/uaccess.h>
#include <linux/uaccess.h>

#define DEVICE_NAME                 "rbt530_dev"  // device name to be created and registered
#define DEVICE_NAME_1                 "rbt530_dev1"
#define DEVICE_NAME_2                 "rbt530_dev2"



/* SOURCES 
https://www.infradead.org/~mchehab/kernel_docs/unsorted/rbtree.html
*/

typedef struct keydata_to_read {
	int key;
	int data;
} keydata_to_read_t;

typedef struct rb_keydata {
	char key;
	char data;
} rb_keydata_t;

typedef struct rb_object{
	int key;
	int data;
	struct rb_node node;
} rb_object_t;


// The actual device struct for first tree
struct rbtree_dev {
	struct cdev cdev;               /* The cdev structure */
	char name[20];                  /* Name of device*/
	char in_string[256];
	struct rb_node* treeCursor;
    struct rb_root mytree;
    spinlock_t spinlockDevice;
    int readOrderDevice;
} *rbtree_devp;

// The actual device struct for second tree
struct rbtree_dev_second {
	struct cdev cdev;               /* The cdev structure */
	char name[20];                  /* Name of device*/
	char in_string[256];
	struct rb_node* treeCursor;
    struct rb_root mytree;
    spinlock_t spinlockDevice;
    int readOrderDevice;
} *rbtree_devp_second;



// sudo cat /proc/kallsyms | grep rbtree_driver_write

// For first device
static dev_t rbtree_dev_number;      
struct class *rbtree_dev_class;           
static struct device *rbtree_dev_device;

// For second device
static dev_t rbtree_dev_number_second;      
struct class *rbtree_dev_class_second;           
static struct device *rbtree_dev_device_second;



static char *user_name = "Kamal Nadesan";  /* the default user name, can be replaced if a new name is attached in insmod command */

module_param(user_name,charp,0000);	//to get parameter from load.sh script to greet the user



/*
* Open rbtree driver
*/
int rbtree_driver_open(struct inode *inode, struct file *file)
{
	struct rbtree_dev *rbtree_devp;

	/* Get the per-device structure that contains this cdev */
	rbtree_devp = container_of(inode->i_cdev, struct rbtree_dev, cdev);

	/* Easy access to cmos_devp from rest of the entry points */
	file->private_data = rbtree_devp;
	printk("\n%s is openning \n", rbtree_devp->name);
	return 0;
}


/*
 * Release rbtree driver
 */
int rbtree_driver_release(struct inode *inode, struct file *file)
{
	struct rbtree_dev *rbtree_devp = file->private_data;
	
	printk("\n%s is closing\n", rbtree_devp->name);
	
	return 0;
}

/*
 * Write to rbtree driver
 */

/*
write: 
if the input object of rb_object_t has a non-zero data field, a node is created and added to 
the rbt530. 
If an object with the same key already exists in the tree, it should be replaced with the 
new one. If the data field is 0, any existing object with the input key is deleted from the table.

*/
// user just passes in struct with key and vaue

struct rb_object *my_search(struct rb_root *root, int key)
  {
  	struct rb_node *node = root->rb_node;

  	while (node) {
  		struct rb_object *curr = container_of(node, struct rb_object, node);
  		//printk("Iterating\n");
  		//printk("Key = %d\n", curr->key);
  		//printk("Data = %d\n", curr->data);

		if(key < curr->key) {
			node = node->rb_left;
		} 
		else if(key > curr->key) {
        	node = node->rb_right;
        } 
        else {
        	// found node
        	printk("Found Node to Delete\n");
            return curr;
        }
	}
	// node does not exist in tree
	return NULL;
  }



int my_insert(struct rb_root *root, struct rb_object *nodeToInsert) {

	// if nodeToInsert->data == 0
	//printk("Key = %d\n", nodeToInsert->key);
	//printk("Data = %d\n", nodeToInsert->data);
	if(nodeToInsert->data == 0) {
		struct rb_object *nodeToDelete = my_search(root, nodeToInsert->key);
  		if (nodeToDelete != NULL) {
  			rb_erase(&(nodeToDelete->node), root);
  			kfree(nodeToDelete);
  			printk("Removed Node\n");
  			return 0;
  		}
	}
    
    struct rb_node **new = &(root->rb_node), *parent = NULL;
    // Figure out where to put new node 
    while (*new) {
        struct rb_object *this = container_of(*new, struct rb_object, node);
        //int result = strcmp(nodeToInsert->keystring, this->keystring);
        parent = *new;
        if(nodeToInsert->key < this->key) {
        	new = &((*new)->rb_left);
        } else if(nodeToInsert->key > this->key) {
        	new = &((*new)->rb_right);
        } else {
        	// node already exists in tree
        	// replace data
        	this->data = nodeToInsert->data;
            return 1;
        }
            
    }

      // Add new node and rebalance tree. 
      rb_link_node(&nodeToInsert->node, parent, new);
      rb_insert_color(&nodeToInsert->node, root);

      return 0;
}
// For kprobe module
//EXPORT_SYMBOL(my_insert);


/*
write: 
if the input object of rb_object_t has a non-zero data field, a node is created and added to 
the rbt530. 
If an object with the same key already exists in the tree, it should be replaced with the 
new one. If the data field is 0, any existing object with the input key is deleted from the table.

*/
// user just passes in struct with key and vaue
// sudo cat /proc/kallsyms | grep rbtree_driver_write

ssize_t rbtree_driver_write(struct file *file, const char *buf,
           size_t count, loff_t *ppos)
{
	// User should provide rb_object
	//printk("DRDRIVER WRITE AROUND = %p\n", (void*) rbtree_driver_write);
	//printk("Write pid = %d\n", current->pid);

	// Declaring this file pointer is for locating the file pointer in the x86 code
	volatile struct file* f = file;


	
	struct rbtree_dev *rbtree_devp = file->private_data;

	// LOCK
	spin_lock(&(rbtree_devp->spinlockDevice));

	
	// TODO: Test count
	printk("Original Count = %zu\n", count);

	struct rb_keydata intermediate;

	int errChkKey = get_user(intermediate.key, (buf));
		if(errChkKey == -EFAULT) {
			printk("Unknown KEY  From Buffer Detected\n");
			spin_unlock(&(rbtree_devp->spinlockDevice));
			return PTR_ERR(buf);
		}
		// add 4 because we want the next int and int has size 4. Adding one is only for chars
		buf = buf + 4;
		int errChkData = get_user(intermediate.data, (buf));
		if(errChkData == -EFAULT) {
			printk("Unknown KEY  From Buffer Detected\n");
			spin_unlock(&(rbtree_devp->spinlockDevice));
			return PTR_ERR(buf);
		}
	//printk("KEY = %d\n", intermediate.key);
	//printk("DATA = %d\n", intermediate.data);

	int keyToInsert = (int) intermediate.key;
	int dataToInsert = (int) intermediate.data;

	if(dataToInsert == 0) {
		// If (data==0) remove corresponding node
		struct rb_object* nodeToDelete = my_search(&(rbtree_devp->mytree), keyToInsert);
		if (nodeToDelete) {
			// the node exists in the tree and now should be deleted
      		rb_erase(&nodeToDelete->node, &(rbtree_devp->mytree));
      		kfree(nodeToDelete);
      		printk("Node Deleted\n");
		} else {
			// Node does not exist in tree and no action should be taken in this while loop iteration
		}
	} else {
		// Insert new node or replace data field of existing node in tree
		struct rb_object* nodeToInsert = kmalloc(sizeof(struct rb_object), GFP_KERNEL);
		if (!nodeToInsert) {
			printk("Bad Kmalloc\n");

			// UNLOCK 
			spin_unlock(&(rbtree_devp->spinlockDevice));
			return -ENOMEM;
		}
		// fill in node 
		nodeToInsert->key = keyToInsert;
		nodeToInsert->data = dataToInsert;
		int result = my_insert(&(rbtree_devp->mytree), nodeToInsert);
		if(result == 0) {
			printk("Node Inserted\n");
		} else {
			printk("Node Exists in Tree\n");
			kfree(nodeToInsert);
		}
	}
		


	// Display Tree
	struct rb_node *node;
	if(rbtree_devp->readOrderDevice == 0) {
		printk("Display Tree In Ascending Order\n");

		for (node = rb_first(&(rbtree_devp->mytree)); node != NULL; node = rb_next(node)) {
    		printk("key=%d\n", rb_entry(node, struct rb_object, node)->key);
    		printk("data=%d\n", rb_entry(node, struct rb_object, node)->data);
    		if(rb_next(node) == NULL) {
    			break;
    		}
    	}

	}
	else {
		printk("Display Tree In DESCENDING Order\n");

    	for (node = rb_last(&(rbtree_devp->mytree)); node != NULL; node = rb_prev(node)) {
    		printk("key=%d\n", rb_entry(node, struct rb_object, node)->key);
    		printk("data=%d\n", rb_entry(node, struct rb_object, node)->data);

    		if(rb_prev(node) == NULL) {
    			break;
    		}

    	}

	}
    
    
    
    
	
	// UNLOCK
	spin_unlock(&(rbtree_devp->spinlockDevice));
	return 0;
}
// For kprobe module
//EXPORT_SYMBOL(rbtree_driver_write);


/*
ioctl: 
The command “read_order” to set up ascending or descending order that objects are to be 
read

If the argument is 0, read calls retrieve the objects in ascending order starting from the object 
with  the  minimal  key. 

It  if  is  1,  read  calls  get  the  objects  in  descending  order  star
ting  from  the object with the maximal key. 

Otherwise, -1 is returned and errno is set to EINVAL.

*/
// TODO: Use unlocked ioctl
long rbtree_driver_unlocked_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_param) {
	// TODO: local read order

	

	struct rbtree_dev *rbtree_devp = file->private_data;

	// LOCK
	spin_lock(&(rbtree_devp->spinlockDevice));


	printk("IN Unlocked_IOTCL\n");
	printk("ioctl number = %d\n", ioctl_num);


	if(ioctl_num == 0) {
		rbtree_devp->readOrderDevice = 0;
	} else if(ioctl_num == 1) {
		rbtree_devp->readOrderDevice = 1;
	} 
	else {
		printk("ioctl must have 0 or 1 as a parameter to set read order or 2 to displaying all contents\n");
		// TODO: search how to set up error call ioctl
		// UNLOCK
		spin_unlock(&(rbtree_devp->spinlockDevice));
		return -EINVAL;
	}

	// UNLOCK
	spin_unlock(&(rbtree_devp->spinlockDevice));
	return 0;

}






/*
 * Read to rbtree driver
 */

/*
read: 
to retrieve the next object in either ascending or descending order (to be set by an ioctl call) from the RB tree. 
  If the RB tree is empty or the next object is null, -1 is returned and errno is set to EINVAL.

  struct rb_node *rb_first(struct rb_root *tree);
  struct rb_node *rb_last(struct rb_root *tree);
  struct rb_node *rb_next(struct rb_node *node);
  struct rb_node *rb_prev(struct rb_node *node);

  typedef struct rb_keydata {
	char key;
	char data;
} rb_keydata_t;

*/
ssize_t rbtree_driver_read(struct file *file, char *buf,
           size_t count, loff_t *ppos)
{
	// TODO: Write object back to user
	// TODO: add cursor to where you are in the tree

	// This file pointer is for referencing the x86 code for the kprobe pre-handler
	volatile struct file* f = file;

	

	
	struct rbtree_dev *rbtree_devp = file->private_data;

	// LOCK
	spin_lock(&(rbtree_devp->spinlockDevice));


	// for debugging x86
	volatile int i = 0;
	i = i + 1;

	// bytes read back to user
	int bytes_read = 0;
	


	if(count != 8) {
		// copies data to user in 8 byte increments

		// UNLOCK
		spin_unlock(&(rbtree_devp->spinlockDevice));
		return -1;
	}

	struct keydata_to_read sendToUser;
	bool cursorHasBeenUsed = true;
	//struct rb_node *node;




	//rbtree_devp->readOrderDevice = 1;
	printk("READ NODES IN TREE KERNELSPACE \n");
	if(rbtree_devp->readOrderDevice == 0) {
		// read in ascending order
		printk("Ascending Order\n");

		if(rbtree_devp->treeCursor == NULL) {
			// treeCursor has not been used yet
			printk("Empty Tree\n");
			rbtree_devp->treeCursor = rb_first(&(rbtree_devp->mytree));
			cursorHasBeenUsed = false;
		}
		
		if(rb_first(&(rbtree_devp->mytree)) == NULL) {
			// tree is empty

			// UNLOCK
			spin_unlock(&(rbtree_devp->spinlockDevice));
			return -EINVAL;
		}

		// update cursor 
		// test the node to read (the one after the current node we are on) to see if it is null
		struct rb_node *testNext = rb_next(rbtree_devp->treeCursor);
		if(testNext == NULL && cursorHasBeenUsed == true) {
			printk("The next node is empty.\n");

			// UNLOCK
			spin_unlock(&(rbtree_devp->spinlockDevice));
			return -1;
		} else {
			// update cursor
			if(cursorHasBeenUsed == true) {
				rbtree_devp->treeCursor = rb_next(rbtree_devp->treeCursor);
			} else {
				// cursor has now been ininitialized
				cursorHasBeenUsed = true;
			}
			// send data to user
			struct rb_node *node = rbtree_devp->treeCursor;
			if(rbtree_devp->treeCursor == NULL) {
				printk("Cursur Is NULL\n");

				// UNLOCK
				spin_unlock(&(rbtree_devp->spinlockDevice));
				return -1;
			}

			sendToUser.key = rb_entry(node, struct rb_object, node)->key;
    		sendToUser.data = rb_entry(node, struct rb_object, node)->data;
    		printk("KEY To send to user = %d\n", sendToUser.key);
    		printk("DATA To send to user = %d\n", sendToUser.data);
    		printk("COUNT READ = %zu", count);

    		int errChkKeyData = copy_to_user(buf, &sendToUser, sizeof(struct keydata_to_read));
			if(errChkKeyData == -EFAULT) {
				printk("Get_User Error\n");
				spin_unlock(&(rbtree_devp->spinlockDevice));
				return PTR_ERR(node);
			}
			// Update Variables
			//count = count - sizeof(struct keydata_to_read);
			bytes_read = bytes_read + sizeof(struct keydata_to_read);

			// UNLOCK
			spin_unlock(&(rbtree_devp->spinlockDevice));

			// Most read functions return the number of bytes put into the buffer
			return bytes_read;
		}
	} 
	else if(rbtree_devp->readOrderDevice == 1) {
		// read in descending order
		printk("Descending Order\n");
			
		if(rbtree_devp->treeCursor == NULL) {
			// treeCursor has not been used yet
			printk("Empty Tree\n");
			cursorHasBeenUsed = false;
			rbtree_devp->treeCursor = rb_last(&(rbtree_devp->mytree));
		}
		if(rb_last(&(rbtree_devp->mytree)) == NULL) {
			// tree is empty

			// UNLOCK
			spin_unlock(&(rbtree_devp->spinlockDevice));
			return -EINVAL;
		}
		// test the node to read (the one after the current node we are on) to see if it is null
		struct rb_node *testPrev = rb_prev(rbtree_devp->treeCursor);
		if(testPrev == NULL && cursorHasBeenUsed == true) {
			printk("The prev node is empty.\n");

			// UNLOCK
			spin_unlock(&(rbtree_devp->spinlockDevice));
			return -EINVAL;
		} else {
			// update cursor
			if(cursorHasBeenUsed == true) {
				rbtree_devp->treeCursor = rb_prev(rbtree_devp->treeCursor);
			} else {
				cursorHasBeenUsed = true;
			}

			//rbtree_devp->treeCursor = rb_prev(rbtree_devp->treeCursor);
			// send data to user
			struct rb_node *node = rbtree_devp->treeCursor;
			sendToUser.key = rb_entry(node, struct rb_object, node)->key;
    		sendToUser.data = rb_entry(node, struct rb_object, node)->data;
    		printk("KEY = %d\n", sendToUser.key);
    		printk("DATA = %d\n", sendToUser.data);
    		printk("COUNT READ = %zu", count);

    		int errChkKeyData = copy_to_user(buf, &sendToUser, sizeof(struct keydata_to_read));
			if(errChkKeyData == -EFAULT) {
				printk("copy_to_user Error\n");

				// UNLOCK
				spin_unlock(&(rbtree_devp->spinlockDevice));
				return PTR_ERR(node);
			}
			// Update Variables
			bytes_read = bytes_read + sizeof(struct keydata_to_read);

			// UNLOCK
			spin_unlock(&(rbtree_devp->spinlockDevice));

			// Most read functions return the number of bytes put into the buffer
			return bytes_read;
		}
	} 
	else {
		printk("ioctl Error\n");

		// UNLOCK
		spin_unlock(&(rbtree_devp->spinlockDevice));
		return -1;

	}
}
// For kprobe module
//EXPORT_SYMBOL(rbtree_driver_read);











/* File operations structure for first device. Defined in linux/fs.h */
static struct file_operations rbtree_fops = {
    .owner		= THIS_MODULE,           /* Owner */
    .open		= rbtree_driver_open,        /* Open method */
    .release	= rbtree_driver_release,     /* Release method */
    .write		= rbtree_driver_write,       /* Write method */
    .read		= rbtree_driver_read,        /* Read method */
    .unlocked_ioctl      = rbtree_driver_unlocked_ioctl
};



/* File operations structure for second device. Defined in linux/fs.h */
static struct file_operations rbtree_fops_second = {
    .owner		= THIS_MODULE,           /* Owner */
    .open		= rbtree_driver_open,        /* Open method */
    .release	= rbtree_driver_release,     /* Release method */
    .write		= rbtree_driver_write,       /* Write method */
    .read		= rbtree_driver_read,        /* Read method */
    .unlocked_ioctl      = rbtree_driver_unlocked_ioctl
};







/*
 * Driver Initialization
 */
int __init rbtree_driver_init(void)
{
	int ret;
	int time_since_boot;


	// Request dynamic allocation of a device major number /
	if (alloc_chrdev_region(&rbtree_dev_number, 0, 1, DEVICE_NAME) < 0) {
			printk(KERN_DEBUG "Can't register device\n"); return -1;
	}

	/* Populate sysfs entries */
	rbtree_dev_class = class_create(THIS_MODULE, DEVICE_NAME);

	if(IS_ERR(rbtree_dev_class)) {
		// TODO: Add unregister_chrdev_region
		printk("class_create(...) ERROR\n");
		// Release the major number /
	    unregister_chrdev_region((rbtree_dev_number), 1);
		return(PTR_ERR(rbtree_dev_class));
	}

	// Allocate memory for the per-device structure */
	rbtree_devp = kmalloc(sizeof(struct rbtree_dev), GFP_KERNEL);
		
	if (!rbtree_devp) {
		printk("Bad Kmalloc\n"); return -ENOMEM;
	}

	spin_lock_init(&(rbtree_devp->spinlockDevice));

	// set default read order value (can be changed with ioctl)
	rbtree_devp->readOrderDevice = 0;

	// Request I/O region /
	sprintf(rbtree_devp->name, DEVICE_NAME);

	// Connect the file operations with the cdev/
	cdev_init(&rbtree_devp->cdev, &rbtree_fops);
	rbtree_devp->cdev.owner = THIS_MODULE;

	memset(rbtree_devp->in_string, 0, 256);

	// Initialize rb_tree root
	rbtree_devp->mytree = RB_ROOT;

	// Initialize treeCursor to be the first element in
	rbtree_devp->treeCursor = NULL;

	// Connect the major/minor number to the cdev /
	ret = cdev_add(&rbtree_devp->cdev, (rbtree_dev_number), 1);

	if (ret) {
		printk("Bad cdev\n");
		return ret;
	}

	// Send uevents to udev, so it'll create /dev nodes /
	rbtree_dev_device = device_create(rbtree_dev_class, NULL, MKDEV(MAJOR(rbtree_dev_number), 0), NULL, DEVICE_NAME);
	
	if(IS_ERR(rbtree_dev_device)) {
		class_destroy(rbtree_dev_class);
		// TODO: add unregister_chrdev_region
		// Release the major number /
	    unregister_chrdev_region((rbtree_dev_number), 1);
		printk("device_create(...) ERROR\n");
		return PTR_ERR(rbtree_dev_device);
	}

	//since on some systems jiffies is a very huge uninitialized value at boot and saved.
	time_since_boot=(jiffies-INITIAL_JIFFIES)/HZ; 
	sprintf(rbtree_devp->in_string, "Hi %s, this machine has been on for %d seconds", user_name, time_since_boot);
	
	

	printk("rbtree driver initialized.\n'%s'\n",rbtree_devp->in_string);
	//printk("rbtree driver initialized.\n'%s'\n",rbtree_devp->mytree);



	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	printk("Initialize SECOND RB_TREE\n");

	if (alloc_chrdev_region(&rbtree_dev_number_second, 0, 1, DEVICE_NAME_2) < 0) {
			printk(KERN_DEBUG "Can't register device\n"); return -1;
	}

	// Populate sysfs entries /
	rbtree_dev_class_second = class_create(THIS_MODULE, DEVICE_NAME_2);

	if(IS_ERR(rbtree_dev_class_second)) {
		// TODO: Add unregister_chrdev_region
		printk("class_create(...) ERROR\n");
		// Release the major number /
	    unregister_chrdev_region((rbtree_dev_number_second), 1);
		return(PTR_ERR(rbtree_dev_class_second));
	}

	// Allocate memory for the per-device structure /
	rbtree_devp_second = kmalloc(sizeof(struct rbtree_dev), GFP_KERNEL);

	memset(rbtree_devp_second, 0, sizeof(struct rbtree_dev));

		
	if (!rbtree_devp_second) {
		printk("Bad Kmalloc\n"); return -ENOMEM;
	}

	spin_lock_init(&(rbtree_devp_second->spinlockDevice));

	// set default read order value (can be changed with ioctl)
	rbtree_devp_second->readOrderDevice = 1;

	// Request I/O region /
	sprintf(rbtree_devp_second->name, DEVICE_NAME_2);

	// Connect the file operations with the cdev/
	cdev_init(&rbtree_devp_second->cdev, &rbtree_fops_second);
	rbtree_devp_second->cdev.owner = THIS_MODULE;

	memset(rbtree_devp_second->in_string, 0, 256);

	// Initialize rb_tree root
	rbtree_devp_second->mytree = RB_ROOT;

	// Initialize treeCursor 
	rbtree_devp_second->treeCursor = NULL;

	// Connect the major/minor number to the cdev /
	ret = cdev_add(&rbtree_devp_second->cdev, (rbtree_dev_number_second), 1);

	if (ret) {
		printk("Bad cdev\n");
		return ret;
	}

	// Send uevents to udev, so it'll create /dev nodes /
	rbtree_dev_device_second = device_create(rbtree_dev_class_second, NULL, MKDEV(MAJOR(rbtree_dev_number_second), 0), NULL, DEVICE_NAME_2);
	
	if(IS_ERR(rbtree_dev_device_second)) {
		class_destroy(rbtree_dev_class_second);
		// TODO: add unregister_chrdev_region
		// Release the major number /
	    unregister_chrdev_region((rbtree_dev_number_second), 1);
		printk("device_create(...) ERROR\n");
		return PTR_ERR(rbtree_dev_device_second);
	}

	//since on some systems jiffies is a very huge uninitialized value at boot and saved.
	time_since_boot=(jiffies-INITIAL_JIFFIES)/HZ; 
	sprintf(rbtree_devp_second->in_string, "Hi %s, this machine has been on for %d seconds", user_name, time_since_boot);
	
	

	printk("rbtree driver SECOND initialized.\n'%s'\n",rbtree_devp_second->in_string);
	
	return 0;
}


/* Driver Exit */
void __exit rbtree_driver_exit(void)
{
	// device_remove_file(rbtree_dev_device, &dev_attr_xxx);

	printk("release first tree\n");
	/* Release the major number */
	unregister_chrdev_region((rbtree_dev_number), 1);

	/* Destroy device */
	device_destroy (rbtree_dev_class, MKDEV(MAJOR(rbtree_dev_number), 0));
	cdev_del(&rbtree_devp->cdev);
	kfree(rbtree_devp);
	
	/* Destroy driver_class */
	class_destroy(rbtree_dev_class);

	//printk("rbtree driver removed.\n");

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	printk("remove second tree\n");
	unregister_chrdev_region((rbtree_dev_number_second), 1);

	// Destroy device /
	device_destroy (rbtree_dev_class_second, MKDEV(MAJOR(rbtree_dev_number_second), 0));
	cdev_del(&rbtree_devp_second->cdev);
	kfree(rbtree_devp_second);
	
	// Destroy driver_class /
	class_destroy(rbtree_dev_class_second);
	
	printk("rbtree driver removed.\n");

	return;

}

module_init(rbtree_driver_init);
module_exit(rbtree_driver_exit);
MODULE_LICENSE("GPL v2");















// USEFUL STUFF POTENTIALLY



