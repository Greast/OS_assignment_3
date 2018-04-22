/* Prototype module for second mandatory DM510 assignment */
#ifndef __KERNEL__
#  define __KERNEL__
#endif
#ifndef MODULE
#  define MODULE
#endif

//#define DEBUG
#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
/* Prototypes - this would normally go in a .h file */

//own header file for buffer, to make code readable
#include "buffer.h"
#include "dm510_ioctl_commands.h"
static int dm510_open( struct inode*, struct file* );
static int dm510_release( struct inode*, struct file* );
static ssize_t dm510_read( struct file*, char*, size_t, loff_t* );
static ssize_t dm510_write( struct file*, const char*, size_t, loff_t* );
long dm510_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);

#define DEVICE_NAME "dm510_dev" /* Dev name as it appears in /proc/devices */
#define MAJOR_NUMBER 254
#define MIN_MINOR_NUMBER 0
#define MAX_MINOR_NUMBER 1

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

#define BUFFER_DEFAULT_SIZE 4096

int dm510_init_module( void ) {
	int i, result;
	result = register_chrdev_region(global_device,DEVICE_COUNT,DEVICE_NAME);
	if(result){
		return rerror(result, "Failed to register chrdev_region.");
	}
	for (i = 0; i < BUFFER_COUNT; i++) {
		result = buffer_init(buffers+i, BUFFER_DEFAULT_SIZE);
		if(result < 0) return rerror(result, "Could not allocate memory for buffer(%d).", i);
	}
	for ( i = 0; i < DEVICE_COUNT; i++) {
		init_waitqueue_head(&devices[i].inq);
		init_waitqueue_head(&devices[i].outq);
		mutex_init(&devices[i].mutex);
		dprintf("Device(%d) = (%d, %d)",i,(i % BUFFER_COUNT), ((i + 1) % BUFFER_COUNT));
		devices[i].read_buffer = buffers + (i % BUFFER_COUNT);
		devices[i].write_buffer = buffers + ((i + 1) % BUFFER_COUNT);
		DEBUG_CODE(printk(""););
		frame_device_setup(devices+i, global_device+i );
	}

	//printk(KERN_INFO "DM510: Hello, faggot, from your device!\n");
	return 0;
}

/* Called when module is unloaded */
void dm510_cleanup_module( void ) {
	int i;
	for(i = 0; i < DEVICE_COUNT ; i++){
		if(devices[i].write_buffer) cdev_del(&devices[i].cdev);
	}
	for(i = 0; i < BUFFER_COUNT ; i++){
		if(buffers[i].buffer) buffer_free(buffers+i);
	}
	unregister_chrdev_region(global_device,DEVICE_COUNT);

	//printk(KERN_INFO "DM510: Module unloaded, you faggot.\n");
}


/* Called when a process tries to open the device file */
static int dm510_open( struct inode *inode, struct file *filp ) {
	struct frame * dev;
	dprintf("Open");
	dev = container_of(inode->i_cdev, struct frame, cdev);
	filp->private_data = dev;
	if(mutex_lock_interruptible(&dev->mutex)){
		return rerror(-ERESTARTSYS, "Mutex lock was interrupted.");
	}

	if (filp->f_mode & FMODE_READ){
		if(dev->nreaders >= max_processes ){
			mutex_unlock(&dev->mutex);
			return rerror(-ERESTARTSYS, "Too many readers, only %d are allowed.", max_processes);
		} else{
			dev->nreaders++;
		}
	}
	if (filp->f_mode & FMODE_WRITE){
		if(dev->nwriters >= 1){
			mutex_unlock(&dev->mutex);
			return rerror(-ERESTARTSYS, "Amount of writers exceeded the allowed capacity of 1.");
		}else{
			dev->nwriters++;
		}

	}
	mutex_unlock(&dev->mutex);

	return nonseekable_open(inode, filp);
}


/* Called when a process closes the device file. */
static int dm510_release( struct inode *inode, struct file *filp ) {
	struct frame * dev = filp->private_data;
	dprintf("Release");
	//scull_p_fasync(-1, filp, 0);
	mutex_lock(&dev->mutex);
	if (filp->f_mode & FMODE_READ && dev->nreaders)
		dev->nreaders--;
	if (filp->f_mode & FMODE_WRITE && dev->nwriters)
		dev->nwriters--;
	mutex_unlock(&dev->mutex);

	return 0;
}


/* Called when a process, which already opened the dev file, attempts to read from it. */
static ssize_t dm510_read( struct file *filp,
    char *buf,      /* The buffer to fill with data     */
    size_t count,   /* The max number of bytes to read  */
    loff_t *f_pos )  /* The offset in the file           */
{
	dprintf("Read.");
	struct frame * dev = filp->private_data;
	char **rp = &dev->read_buffer->rp;
	char **wp = &dev->read_buffer->wp;

	if (mutex_lock_interruptible(&dev->mutex)){
		return rerror(-ERESTARTSYS, "Mutex lock was interrupted by outside source.");
	}



	while (*rp == *wp) {
		mutex_unlock(&dev->mutex);
		if (filp->f_flags & O_NONBLOCK){
			return rerror(-EAGAIN, "File-pointer could not be blocked.");
		}
		dprintf("Read Sleeping.");
		if(wait_event_interruptible(dev->inq,(*rp != *wp))){
			return rerror(-EAGAIN, "Reader was interrupted while sleeping.");
		}
		dprintf("Reader Awoken.");
		if(mutex_lock_interruptible(&dev->mutex)){
			return rerror(-ERESTARTSYS, "Mutex lock was interrupted by outside source.");
		}
	}
	count = buffer_read(dev->read_buffer,buf,count);
	mutex_unlock (&dev->mutex);
	wake_up_interruptible(&dev->outq);
	dprintf("Read : Done.");
	return count;
}

/* Called when a process writes to dev file */
static ssize_t dm510_write( struct file *filp,
    const char *buf,/* The buffer to get data from      */
    size_t count,   /* The max number of bytes to write */
    loff_t *f_pos )  /* The offset in the file           */
{

	struct frame * dev = filp->private_data;
	dprintf("Write.| wp = %d, rp = %d" ,
	dev->write_buffer->wp - dev->write_buffer->buffer,
	dev->write_buffer->rp - dev->write_buffer->buffer);
	if (mutex_lock_interruptible(&dev->mutex))
		return rerror(-ERESTARTSYS, "Mutex lock was interrupted by outside source.");

	if(count > buffers->size){
		return rerror(-EMSGSIZE, "Message exceeded the size of the buffer.");
	}

	while (buffer_free_space(dev->write_buffer) < count) {
		//DEFINE_WAIT(wait);

		mutex_unlock(&dev->mutex);
		if (filp->f_flags & O_NONBLOCK){
			return rerror(-EAGAIN, "File-pointer could not be blocked.");
		}
		if(wait_event_interruptible(dev->outq,
			(buffer_free_space(dev->write_buffer) >= count))){
			return rerror(-EAGAIN, "Writer was interrupted while sleeping.");
		}

		if (mutex_lock_interruptible(&dev->mutex))
			return rerror(-ERESTARTSYS, "Mutex lock was interrupted by outside source.");
	}
	count = buffer_write(dev->write_buffer,(char*)buf,count);
	dprintf("Write : Waking Reader. | wp = %d, rp = %d" ,
	dev->write_buffer->wp - dev->write_buffer->buffer,
	dev->write_buffer->rp - dev->write_buffer->buffer);
	int i;
	for(i = 0 ; i < DEVICE_COUNT ; i++){
			wake_up_interruptible(&devices[i].inq);
	}
	//wake_up_interruptible(&dev->inq);

	mutex_unlock (&dev->mutex);
	dprintf("Write : Done.");
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
			for(i = 0 ; i < BUFFER_COUNT ; i++){
				int used_space = buffers[i].size - buffer_free_space(buffers+i);
				if(used_space > arg) {
					return rerror(-EINVAL, "Buffer(%d) has %lu amount of used space, cannot be reduced to size %lu.", i, used_space, arg);
				}
			}
			for(i = 0 ; i < BUFFER_COUNT ; i++) {
				buffer_resize(buffers+i,arg);
			}
		}
		break;

		case GET_MAX_NR_PROC:
		return max_processes;

		case SET_MAX_NR_PROC:
		max_processes = arg;
		break;

		case GET_BUFFER_FREE_SPACE:
		return buffer_free_space(buffers+arg);

		case GET_BUFFER_USED_SPACE:
		return buffers[arg].size - buffer_free_space(buffers+arg);
	}


	return 0; //has to be changed
}

module_init( dm510_init_module );
module_exit( dm510_cleanup_module );

MODULE_AUTHOR( "Jonas Ingerslev Soerensen, Jeff Gyldenbrand, Simon Dradrach Joergensen" );
MODULE_LICENSE( "GPL" );
