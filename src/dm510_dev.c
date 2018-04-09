/* Prototype module for second mandatory DM510 assignment */
#ifndef __KERNEL__
#  define __KERNEL__
#endif
#ifndef MODULE
#  define MODULE
#endif

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/wait.h>
/* #include <asm/uaccess.h> */
#include <linux/uaccess.h>
#include <linux/semaphore.h>
/* #include <asm/system.h> */
#include <asm/switch_to.h>
/* Prototypes - this would normally go in a .h file */
static int dm510_open( struct inode*, struct file* );
static int dm510_release( struct inode*, struct file* );
static ssize_t dm510_read( struct file*, char*, size_t, loff_t* );
static ssize_t dm510_write( struct file*, const char*, size_t, loff_t* );
long dm510_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);

#define DEVICE_NAME "dm510_dev" /* Dev name as it appears in /proc/devices */
#define MAJOR_NUMBER 254
#define MIN_MINOR_NUMBER 0
#define MAX_MINOR_NUMBER 1

#define DEVICE_COUNT 2
/* end of what really should have been in a .h file */

/* file operations struct */
static struct file_operations dm510_fops = {
	.owner   = THIS_MODULE,
	.read    = dm510_read,
	.write   = dm510_write,
	.open    = dm510_open,
	.release = dm510_release,
        .unlocked_ioctl   = dm510_ioctl
};

struct scull_pipe {
        wait_queue_head_t inq, outq;       /* read and write queues */
        char *buffer, *end;                /* begin of buf, end of buf */
        int buffersize;                    /* used in pointer arithmetic */
        char *rp, *wp;                     /* where to read, where to write */
        int nreaders, nwriters;            /* number of openings for r/w */
        struct fasync_struct *async_queue; /* asynchronous readers */
        struct mutex mutex;              /* mutual exclusion semaphore */
        struct cdev cdev;                  /* Char device structure */
};

struct frame {
	struct cdev device;

}

/* called when module is loaded */
int dm510_init_module( void ) {

	/*
	{
		int i, result;

		result = register_chrdev_region(firstdev, scull_p_nr_devs, "scullp");
		if (result < 0) {
			printk(KERN_NOTICE "Unable to get scullp region, error %d\n", result);
			return 0;
		}
		scull_p_devno = firstdev;
		scull_p_devices = kmalloc(scull_p_nr_devs * sizeof(struct scull_pipe), GFP_KERNEL);
		if (scull_p_devices == NULL) {
			unregister_chrdev_region(firstdev, scull_p_nr_devs);
			return 0;
		}
		memset(scull_p_devices, 0, scull_p_nr_devs * sizeof(struct scull_pipe));
		for (i = 0; i < scull_p_nr_devs; i++) {
			init_waitqueue_head(&(scull_p_devices[i].inq));
			init_waitqueue_head(&(scull_p_devices[i].outq));
			mutex_init(&scull_p_devices[i].mutex);
			scull_p_setup_cdev(scull_p_devices + i, i);
		}
	#ifdef SCULL_DEBUG
		proc_create("scullpipe", 0, NULL, &scull_read_p_mem_proc_fops);
	#endif
		return scull_p_nr_devs;
	}
	*/

	/* initialization code belongs here */

	printk(KERN_INFO "DM510: Hello, faggot, from your device!\n");
	return 0;
}

/* Called when module is unloaded */
void dm510_cleanup_module( void ) {


	/* clean up code belongs here */

	printk(KERN_INFO "DM510: Module unloaded, you faggot.\n");
}


/* Called when a process tries to open the device file */
static int dm510_open( struct inode *inode, struct file *filp ) {

	struct frame *dev;

	dev = container_of(inode->i_cdev, struct frame, device);
	filp->private_data = dev;


	printk(KERN_INFO "open.\n");
	/* device claiming code belongs here */

	return 0;
}


/* Called when a process closes the device file. */
static int dm510_release( struct inode *inode, struct file *filp ) {
	
	printk(KERN_INFO "release.\n");
	/* device release code belongs here */

	return 0;
}


/* Called when a process, which already opened the dev file, attempts to read from it. */
static ssize_t dm510_read( struct file *filp,
    char *buf,      /* The buffer to fill with data     */
    size_t count,   /* The max number of bytes to read  */
    loff_t *f_pos )  /* The offset in the file           */
		printk(KERN_INFO "read.\n");
{

	/* read code belongs here */

	return 0; //return number of bytes read
}


/* Called when a process writes to dev file */
static ssize_t dm510_write( struct file *filp,
    const char *buf,/* The buffer to get data from      */
    size_t count,   /* The max number of bytes to write */
    loff_t *f_pos )  /* The offset in the file           */
		printk(KERN_INFO "write.\n");
{

	/* write code belongs here */

	return 0; //return number of bytes written
}

/* called by system call icotl */
long dm510_ioctl(
    struct file *filp,
    unsigned int cmd,   /* command passed from the user */
    unsigned long arg ) /* argument of the command */
{
	/* ioctl code belongs here */
	printk(KERN_INFO "DM510: ioctl called.\n");

	return 0; //has to be changed
}

module_init( dm510_init_module );
module_exit( dm510_cleanup_module );

MODULE_AUTHOR( "Your names here!" );
MODULE_LICENSE( "GPL" );
