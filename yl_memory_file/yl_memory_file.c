#include "yl_memory_file.h"

#ifdef __KERNEL__

#include <linux/module.h>   /* Needed by all modules */
#include <linux/kernel.h>
#include <linux/slab.h>

#include <asm/uaccess.h>

#define assert(expr)
#define fmin(x,y) min(x,y)

//static const unsigned kalloc_flags = GFP_USER;
static const unsigned kalloc_flags = GFP_KERNEL;

static inline void *malloc(size_t size)
{
	void *ptr = kmalloc(size, kalloc_flags);
	return ptr;
}

static inline void *realloc(void *ptr, size_t size)
{
	ptr = krealloc(ptr, size, kalloc_flags);
	return ptr;
}

static inline void free(void *ptr)
{
	kfree(ptr);
}

#else // __KERNEL__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#define copy_from_user memcpy
#define copy_to_user   memcpy

#endif // __KERNEL__

struct yl_memory_file {
   char *buffer;
   long long buffer_size;
   long long written;
};

static char *do_ylmf_alloc(long long count) {
   char *buffer;

   assert(count > 0);

   buffer = (char *) malloc(count);
   assert(NULL != buffer);

   return buffer;
}

static char *do_ylmf_realloc(char *buffer, long long count) {
   assert(count > 0);

   buffer = (char *) realloc(buffer, count);
   assert(NULL != buffer);

   return buffer;
}

static void do_ylmf_free(void **buffer) {
   if (NULL != buffer) {
      if (NULL != *buffer) {
         free(*buffer);

         *buffer = NULL;
      }
   }
}

static void do_ylfm_memclear(char *buffer, long long count) {
   assert(NULL != buffer);

   if (NULL != buffer) {
      memset(buffer, 0, count);
   }
}

static struct yl_memory_file *do_ylmf_create(unsigned object_size) {
   struct yl_memory_file *instance;

   assert(object_size >= sizeof (struct yl_memory_file));

   instance = (struct yl_memory_file *) do_ylmf_alloc(object_size);

   if (NULL != instance) {
      do_ylfm_memclear((char *) instance, object_size);
   }

   return instance;
}

struct yl_memory_file *ylmf_create_with_buffer_size(long long buffer_size) {
   unsigned object_size;
   struct yl_memory_file *instance;
   char *buffer;
   
   object_size = sizeof (struct yl_memory_file);

   instance = do_ylmf_create(object_size);

   if (NULL != instance) {
      buffer = do_ylmf_alloc(buffer_size);
      if (NULL != buffer) {
         instance->buffer = buffer;
         instance->buffer_size = buffer_size;
      } else {
         do_ylmf_free((void **) &instance);
         assert(NULL == instance);
      }
   }
   return instance;
}

void ylmf_destroy(struct yl_memory_file **instance) {
   assert(NULL != instance);

   if (NULL != instance) {
      if (NULL != *instance) {
         do_ylmf_free((void **) &(*instance)->buffer);
         assert(NULL == (*instance)->buffer);
         do_ylmf_free((void **) instance);
         assert(NULL == *instance);
      }
   }
}

long long ylmf_get_buffer_size(struct yl_memory_file *instance) {
   long long buffer_size;

   assert(NULL != instance);

   buffer_size = 0;
   if (NULL != instance) {
      buffer_size = instance->buffer_size;
   }
   return buffer_size;
}

int ylmf_set_buffer_size(struct yl_memory_file *instance, long long buffer_size) {
   int result;

   assert(NULL != instance);
   assert(buffer_size > 0);

   result = -1;
   if (NULL != instance) {
      char *buffer = do_ylmf_realloc(instance->buffer, buffer_size);
      if (NULL != buffer) {
         instance->buffer = buffer;
         instance->buffer_size = buffer_size;

         result = 0;
      }
   }

   return result;
}

long long ylmf_get_buffer_written(struct yl_memory_file *instance) {
   unsigned written = 0;
   
   assert(NULL != instance);
   
   if (NULL != instance) {
      written = instance->written;
   }
   
   return written;
}

static inline long long get_available(struct yl_memory_file *instance) {
   long long available = instance->buffer_size - instance->written;
   return available;
}

unsigned ylmf_write(struct yl_memory_file *instance, const char *data, unsigned count, long long *offset) {
   unsigned written;

   assert(NULL != instance);
   
   written = 0;
   if ((NULL != instance) && (NULL != offset)) {
      long long available;

      assert(NULL != instance->buffer);
      assert(instance->buffer_size >= instance->written);

      available = get_available(instance);
//      printk(KERN_INFO "available: %ll\n", available);
      if (available < count) {
         int result = 0;
         long long new_buffer_size = instance->written + count;

	      result = ylmf_set_buffer_size(instance, new_buffer_size);
         if (result >= 0) {
            available = get_available(instance);
         }
         else {
            available = -1;
         }
      }

      if (available > 0) {
         written = fmin(count, available);
         copy_from_user(instance->buffer + instance->written, data, written);
         instance->written += written;

         *offset += written;
      }
   }

   return written;
}

unsigned ylmf_read(struct yl_memory_file *instance, char *buffer, unsigned count, long long *offset) {
   unsigned read;

   assert(NULL != instance);
   assert(NULL != offset);
   
   read = 0;
   if ((NULL != instance) && (NULL != offset)) {
      assert(NULL != instance->buffer);

      if (instance->written > *offset) {
         long long available = instance->written - *offset;

         read = fmin(count, available);
         copy_to_user(buffer, instance->buffer + *offset, read);
         *offset += read;
      }
   }
   
   return read;
}
