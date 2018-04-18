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
#include "scull.h"


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
#define BUFFER_COUNT 2
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
static struct buffer buffers[BUFFER_COUNT];
static size_t max_processes = 10;

dev_t global_device = MKDEV(MAJOR_NUMBER,MIN_MINOR_NUMBER);




static int frame_device_setup(struct frame * dev, dev_t device){
	cdev_init(&dev->cdev, &dm510_fops);
	dev->cdev.owner = THIS_MODULE;
	return cdev_add(&dev->cdev, device, 1);
};

#define BUFFER_DEFAULT_SIZE 2048

int dm510_init_module( void ) {
	int i, result;
	result = register_chrdev_region(global_device,DEVICE_COUNT,DEVICE_NAME);
	if(result){
		printk(KERN_NOTICE "Unable to get device region, error %d\n", result);
		return 0;
	}
	for (i = 0; i < BUFFER_COUNT; i++) {
		buffer_init(buffers+i,BUFFER_DEFAULT_SIZE);
	}
	for ( i = 0; i < DEVICE_COUNT; i++) {
		init_waitqueue_head(&devices[i].inq);
		init_waitqueue_head(&devices[i].outq);
		mutex_init(&devices[i].mutex);
		devices[i].read_buffer = buffers + (i % BUFFER_COUNT);
		devices[i].write_buffer = buffers + ((i + 1) % BUFFER_COUNT);
		frame_device_setup(devices+i, global_device+i );
	}

	printk(KERN_INFO "DM510: Hello, faggot, from your device!\n");
	return 0;
}

/* Called when module is unloaded */
void dm510_cleanup_module( void ) {
	int i;
	for(i = 0; i < DEVICE_COUNT ; i++){
		if(devices[i].write_buffer) cdev_del(&devices[i].cdev);
	}
	for(i = 0; i < BUFFER_COUNT ; i++){
		buffer_free(buffers+i);
	}
	unregister_chrdev_region(global_device,DEVICE_COUNT);

	printk(KERN_INFO "DM510: Module unloaded, you faggot.\n");
}


/* Called when a process tries to open the device file */
static int dm510_open( struct inode *inode, struct file *filp ) {
	struct frame * dev;
	dev = container_of(inode->i_cdev, struct frame, cdev);
	filp->private_data = dev;
	if(mutex_lock_interruptible(&dev->mutex))
		return -ERESTARTSYS;

	if (filp->f_mode & FMODE_READ)
		dev->nreaders++;
	if (filp->f_mode & FMODE_WRITE)
		dev->nwriters++;
	mutex_unlock(&dev->mutex);


	printk(KERN_INFO "open.\n");
	/* device claiming code belongs here */

	return nonseekable_open(inode, filp);
}


/* Called when a process closes the device file. */
static int dm510_release( struct inode *inode, struct file *filp ) {
	struct frame * dev = filp->private_data;
	mutex_lock(&dev->mutex);
	if (filp->f_mode & FMODE_READ)
		dev->nreaders--;
	if (filp->f_mode & FMODE_WRITE)
		dev->nwriters--;
	if (dev->nreaders + dev->nwriters == 0) {
		buffer_free(dev->write_buffer);
		buffer_free(dev->read_buffer);
	}
	mutex_unlock(&dev->mutex);
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
	printk(KERN_INFO "read.\n");
	struct frame * dev = filp->private_data;

	if (mutex_lock_interruptible(&dev->mutex))
		return -ERESTARTSYS;

	if (count > buffer_write_space(dev->read_buffer)) {
		printk(KERN_INFO "loop.\n");
		mutex_unlock(&dev->mutex); /* release the lock */
		if (filp->f_flags & O_NONBLOCK)
			return -EAGAIN;
		if (wait_event_interruptible(dev->inq, (count > buffer_write_space(dev->read_buffer))))
			return -ERESTARTSYS; /* signal: tell the fs layer to handle it */
		if (mutex_lock_interruptible(&dev->mutex))
			return -ERESTARTSYS;
	}
	printk(KERN_INFO "copy.\n");
	count = buffer_read(dev->read_buffer,buf,count);
	mutex_unlock (&dev->mutex);
	wake_up_interruptible(&dev->outq);
	printk(KERN_INFO "bytes : %lu .\n", count);
	return count;
}


/* Called when a process writes to dev file */
static ssize_t dm510_write( struct file *filp,
    const char *buf,/* The buffer to get data from      */
    size_t count,   /* The max number of bytes to write */
    loff_t *f_pos )  /* The offset in the file           */
{

	struct frame * dev = filp->private_data;

	if (mutex_lock_interruptible(&dev->mutex))
		return -ERESTARTSYS;

	while (count > buffer_write_space(dev->write_buffer)) {
		mutex_unlock(&dev->mutex); /* release the lock */
		if (filp->f_flags & O_NONBLOCK)
			return -EAGAIN;
		if (wait_event_interruptible(dev->inq, (count > buffer_write_space(dev->write_buffer))))
			return -ERESTARTSYS; /* signal: tell the fs layer to handle it */
		if (mutex_lock_interruptible(&dev->mutex))
			return -ERESTARTSYS;
	}
	count = buffer_write(dev->write_buffer,buf,count);
	mutex_unlock (&dev->mutex);
	wake_up_interruptible(&dev->outq);
	return count;
}


#define GET_BUFFER_SIZE 0
#define SET_BUFFER_SIZE 1
#define GET_MAX_NR_PROC 2
#define SET_MAX_NR_PROC 3

/* called by system call icotl */
long dm510_ioctl(
    struct file *filp,
    unsigned int cmd,   /* command passed from the user */
    unsigned long arg ) /* argument of the command */
{
	switch(cmd){
		case GET_BUFFER_SIZE:
		return buffers->size;

		case SET_BUFFER_SIZE:{
			int i;
			for(i = 0 ; i < BUFFER_COUNT ; i++) buffer_resize(buffers+i,arg);
		}
		break;

		case GET_MAX_NR_PROC:
		return max_processes;

		case SET_MAX_NR_PROC:
		max_processes = arg;
		break;
	}


	/* ioctl code belongs here */
	printk(KERN_INFO "DM510: ioctl called.\n");

	return 0; //has to be changed
}

module_init( dm510_init_module );
module_exit( dm510_cleanup_module );

MODULE_AUTHOR( "Jonas Ingerslev Soerensen, Jeff Gyldenbrand, Simon Dradrach Joergensen" );
MODULE_LICENSE( "GPL" );
