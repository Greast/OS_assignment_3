#include <linux/slab.h>
#include <linux/errno.h>
struct buffer{
	char *buffer;
  size_t size;
	char *rp, *wp;
  struct mutex mutex;
};
//In accordance with IOCTL declaraion section 1, the buffer shall support scaleing.
int buffer_resize(struct buffer * buf, size_t size){
	char *new_buffer = krealloc(buf->buffer, size, GFP_KERNEL);
	if(!new_buffer && size) return -ENOMEM;
	buf->rp = buf->wp = buf->buffer = new_buffer,
	buf->size = size;
	return 0;
}

int buffer_init(struct buffer * buf, size_t size){
	buf->buffer = NULL;
	return buffer_resize(buf,size);
}

struct buffer * buffer(size_t size){
  struct buffer * buf = kmalloc(sizeof(*buf), GFP_KERNEL);
  buffer_init(buf,size);
  return buf;
}

int buffer_free(struct buffer * buf){
	int result;
	result = buffer_resize(buf,0);
	return result;
}

size_t buffer_write_space(struct buffer * buf){
  if (buf->rp == buf->wp)
    return buf->size - 1;
  return ((buf->rp + buf->size - buf->wp) % buf->size) - 1;
}

size_t buffer_io(struct buffer * buf, const char * seq, size_t size, unsigned long (*copy_function)(void*,const void*,unsigned long)){
  mutex_lock(&buf->mutex);
  if(buf->rp > buf->wp){
    size = min((size_t)(buf->wp - buf->rp) - 1, size);
    copy_function(buf->wp,seq,size);
    buf->wp += size;
    mutex_unlock (&buf->mutex);
    return size;

  } else if(buf->rp == buf->wp){
		mutex_unlock (&buf->mutex);
		return 0;
	} else {

    const size_t a = (buf->buffer + buf->size) - buf->rp, b = buf->rp - buf->buffer - 1;

    size_t new_size = min(a,size);
    copy_function(buf->wp,seq,a);
    buf->wp += new_size;
    size-=new_size;

    if( size > 0 ){
      new_size = min(b, size);
      copy_function(buf->buffer,seq,new_size);
      buf->wp = buf->buffer + new_size;
      mutex_unlock (&buf->mutex);
      return a + new_size;
    }
    mutex_unlock (&buf->mutex);
    return new_size;
  }
}

size_t buffer_write(struct buffer * buf, const char * seq, size_t size){
  return buffer_io(buf,seq,size,copy_from_user);
}

size_t buffer_read(struct buffer * buf, const char * seq, size_t size){
  return buffer_io(buf,seq,size,copy_to_user);
}
