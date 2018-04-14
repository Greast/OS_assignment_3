/* Prototype module for second mandatory DM510 assignment */
#ifndef __KERNEL__
#  define __KERNEL__
#endif
#ifndef MODULE
#  define MODULE
#endif

	#include <linux/module.h>
	#include <linux/moduleparam.h>

	#include <linux/kernel.h>	/* printk(), min() */
	#include <linux/slab.h>		/* kmalloc() */
	#include <linux/sched.h>
	#include <linux/fs.h>		/* everything... */
	#include <linux/proc_fs.h>
	#include <linux/errno.h>	/* error codes */
	#include <linux/types.h>	/* size_t */
	#include <linux/fcntl.h>
	#include <linux/poll.h>
	#include <linux/cdev.h>
	#include <linux/seq_file.h>
	#include <linux/sched/signal.h>
	#include <linux/uaccess.h>	/* copy_*_user */

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

#include "buffer.h"



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
  .unlocked_ioctl  = dm510_ioctl
};

struct frame {
	wait_queue_head_t inq, outq;       /* read and write queues */
	struct buffer * read_buffer;
	struct buffer * write_buffer;
	int nreaders, nwriters;            /* number of openings for r/w */
	struct fasync_struct *async_queue; /* asynchronous readers */
	struct mutex mutex;              /* mutual exclusion semaphore */
	struct cdev cdev;
};

static struct frame devices[DEVICE_COUNT];
static struct buffer buffers[2];
static dev_t global_device;

static int frame_device_setup(struct frame * dev, dev_t device){
	cdev_init(&dev->cdev, &dm510_fops);
	dev->cdev.owner = THIS_MODULE;
	return cdev_add(&dev->cdev, device, 1);
};

int dm510_init_module( void ) {
	int i, result;
	size_t size;
	global_device = MAJOR_NUMBER;
	result = register_chrdev_region(global_device,DEVICE_COUNT,DEVICE_NAME);
	if(!result){
		printk(KERN_NOTICE "Unable to get device region, error %d\n", result);
		return 0;
	}
	for ( i = 0; i < DEVICE_COUNT; i++) {
		init_waitqueue_head(&devices[i].inq);
		init_waitqueue_head(&devices[i].outq);
		mutex_init(&devices[i].mutex);
		size = sizeof(buffers) / sizeof(*buffers);
		devices[i].read_buffer = buffers + (i % size);
		devices[i].write_buffer = buffers + ((i + 1) % size);
		frame_device_setup(devices+i, global_device+i );
	}

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
{
	return 0; //return number of bytes read
}


/* Called when a process writes to dev file */
static ssize_t dm510_write( struct file *filp,
    const char *buf,/* The buffer to get data from      */
    size_t count,   /* The max number of bytes to write */
    loff_t *f_pos )  /* The offset in the file           */
{

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

MODULE_AUTHOR( "Jonas Ingerslev Soerensen, Jeff Gyldenbrand, Simon Dradrach Joergensen" );
MODULE_LICENSE( "GPL" );
