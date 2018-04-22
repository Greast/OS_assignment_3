#ifndef DM510_DEBUG_H
#define DM510_DEBUG_H
  #include <linux/slab.h>
  #ifdef DEBUG
  # define DEBUG_CODE(code) do {code} while (0);
  #else
  # define DEBUG_CODE(code) do {} while (0)
  #endif
  # define dprintf(...) DEBUG_CODE(printk("%s/%d : ",__FILE__,__LINE__);printk(__VA_ARGS__ );)
  # define rerror(errno,...) ({dprintf(KERN_ERR "Error (%d) : ",errno) ; DEBUG_CODE(printk(KERN_CONT __VA_ARGS__ );); errno;})
#endif /* end of include guard: DM510_DEBUG_H */
