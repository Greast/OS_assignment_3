#ifndef DM510_BUFFER_H
#define DM510_BUFFER_H

#include <linux/slab.h>
#include <linux/errno.h>
#include "debug.h"

struct buffer{
	char *buffer;
  size_t size;
	char *rp, *wp;
  struct mutex mutex;
};

// return how much space we can write to. If read- and write-pointer is at same spot -> it means
// the whole buffer can be written to. Else, calculate the write-space between them.
size_t buffer_free_space(struct buffer * head){
  if (head->rp == head->wp)
    return head->size - 1;
  return ((head->rp + head->size - head->wp) % head->size) - 1;
}

//In accordance with IOCTL declaraion section 1, the buffer shall support scaleing.
int buffer_resize(struct buffer * head, size_t size){
	void * pointer;
	mutex_lock(&head->mutex);

	pointer = kmalloc(size*sizeof(*head->buffer),GFP_KERNEL);

	if(!pointer) return rerror(-ENOMEM);
	if(head-> wp == head-> rp){
		head->wp = head->rp = pointer;

	} else if(head-> wp > head-> rp){
		size = head->wp - head->rp;
		memcpy(pointer,head->rp,size);
		head->wp = pointer + size;
		head->rp = pointer;

	} else {
		size = (head->buffer + head->size) - head->rp;
		memcpy(pointer,head->rp,size);
		head->rp = pointer;
		memcpy(head->rp + size, head->buffer, head->wp - head->buffer);
		head->wp = (head->rp + size) + (head->wp - head->buffer);

	}
	head->buffer = pointer;
	kfree(head->buffer);
	mutex_unlock (&head->mutex);
	return 0;
}

// initialise a new buffer of given size.
int buffer_init(struct buffer * head, size_t size){
	void * pointer;
	DEBUG_CODE(if(head->buffer) dprintf("To prevent memory leaks, the buffer pointer should be NULL."););

	pointer = kmalloc(size * sizeof(*head->buffer),GFP_KERNEL);

	if(!pointer) return rerror(-ENOMEM);

	head->wp = head->rp = head->buffer = pointer;
	head->size = size;

	return 0;
}

// allocate memory for buffer
struct buffer * buffer(size_t size){
  struct buffer * head = kmalloc(sizeof(*head), GFP_KERNEL);
	buffer_init(head,size);
  return head;
}

// free buffer by resizing to zero. This is possible because if
// krealloc's new_size is 0 and p is not a NULL pointer, the object pointed to is freed.
int buffer_free(struct buffer * head){
	kfree(head->buffer);
	head->buffer = NULL;
	return 0;
}

size_t buffer_write(struct buffer * buf, char * seq, size_t size){
	size_t new_size;
	mutex_lock(&buf->mutex);
	//dprintf("(%lu).wp : %lu" , (size_t)buf, (size_t)(buf->wp - buf->buffer));
	if(buf->wp < buf->rp){
		new_size = min((size_t)(buf->wp - buf->rp) - 1, size);
		copy_from_user(buf->wp,seq,new_size);
		buf->wp += new_size;

	}else{
		const size_t a = (buf->buffer + buf->size) - buf->wp;
		const size_t b = (buf->rp - buf->buffer) % buf->size;
		new_size = min(a,size);
		copy_from_user(buf->wp,seq,new_size);
		size -= new_size;
		if(0 < size){
			new_size = min(b,size);
			buf->wp = buf->buffer;
			copy_from_user(buf->wp,seq,new_size - 1);
		}
		buf->wp += new_size;
	}
	//dprintf("-> %lu\n" , (size_t)(buf->wp - buf->buffer));
	mutex_unlock (&buf->mutex);
	return new_size;
}
size_t buffer_read(struct buffer * buf, char * seq, size_t size){
	size_t new_size = 0;
	mutex_lock(&buf->mutex);
	//dprintf("(%lu).rp : %lu" , (size_t)buf, (size_t)(buf->rp - buf->buffer));
	if(buf->rp < buf->wp){
		new_size = min((size_t)(buf->wp - buf->rp), size);
		copy_to_user(seq,buf->rp,new_size);
		buf->rp += new_size;

	} else {
		const size_t a = (buf->buffer + buf->size) - buf->rp;
		const size_t b = buf->rp - buf->buffer;

		new_size = min(a,size);
		copy_to_user(seq,buf->rp,new_size);
		size -= new_size;
		if(0 < size){
			new_size = min(b,new_size);
			buf->rp = buf->buffer;
			copy_to_user(seq,buf->rp,new_size);
		}
		buf->rp += new_size;

	}
	//dprintf("-> %lu\n" , buf->rp - buf->buffer);
	mutex_unlock (&buf->mutex);
	return new_size;
}
#endif /* end of include guard: DM510_BUFFER_H */
