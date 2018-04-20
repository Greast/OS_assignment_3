#include <linux/slab.h>
#include <linux/errno.h>

struct buffer{
	char *buffer;
  size_t size;
	char *rp, *wp;
  struct mutex mutex;
};

//In accordance with IOCTL declaraion section 1, the buffer shall support scaleing.
int buffer_resize(struct buffer * head, size_t size){
	char *new_buffer = krealloc(head->buffer, size, GFP_KERNEL);
	if(!new_buffer && size) return -ENOMEM;
	head->rp = head->wp = head->buffer = new_buffer,
	head->size = size;
	return 0;
}


// initialise a new buffer of given size.
int buffer_init(struct buffer * head, size_t size){
	head->buffer = NULL;
	return buffer_resize(head,size);
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
	int result;
	result = buffer_resize(head,0);
	return result;
}

// return how much space we can write to. If read- and write-pointer is at same spot -> it means 
// the whole buffer can be written to. Else, calculate the write-space between them.
size_t buffer_write_space(struct buffer * head){
  if (head->rp == head->wp)
    return head->size - 1;
  return ((head->rp + head->size - head->wp) % head->size) - 1;
}

