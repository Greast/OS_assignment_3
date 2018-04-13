#include <linux/slab.h>
struct buffer{
	char *buffer;
  size_t size;
	char *rp, *wp;
  struct mutex mutex;
};

struct buffer * buffer(size_t size){
  struct buffer * buf = kmalloc(sizeof(*buf), GFP_KERNEL);
  buf->size = size;
  buf->rp = buf->wp = buf->buffer = kmalloc(buf->size * sizeof(*buf->buffer), GFP_KERNEL);
  return buf;
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
