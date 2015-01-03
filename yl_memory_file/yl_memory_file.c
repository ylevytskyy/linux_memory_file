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
   unsigned buffer_size;
   unsigned written;
   unsigned read;
};

static char *do_ylmf_alloc(unsigned count) {
   char *buffer;

   assert(count > 0);

   buffer = (char *) malloc(count);
   assert(NULL != buffer);

   return buffer;
}

static char *do_ylmf_realloc(char *buffer, unsigned count) {
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

static void do_ylfm_memclear(char *buffer, unsigned count) {
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

struct yl_memory_file *ylmf_create_with_buffer_size(unsigned buffer_size) {
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

unsigned ylmf_get_buffer_size(struct yl_memory_file *instance) {
   unsigned buffer_size;

   assert(NULL != instance);

   buffer_size = 0;
   if (NULL != instance) {
      buffer_size = instance->buffer_size;
   }
   return buffer_size;
}

int ylmf_set_buffer_size(struct yl_memory_file *instance, unsigned buffer_size) {
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

unsigned ylmf_get_buffer_available(struct yl_memory_file *instance) {
   unsigned available = 0;
   
   assert(NULL != instance);
   
   if (NULL != instance) {
      available = instance->written - instance->read;
   }
   
   return available;
}

static unsigned try_to_move(struct yl_memory_file *instance, unsigned count) {
   unsigned available;
   
   available = instance->buffer_size - instance->written;
   if (available < count) {
      unsigned bytes_to_move;
      
      bytes_to_move = ylmf_get_buffer_available(instance);
      memmove(instance->buffer, instance->buffer + instance->read, bytes_to_move);
      
      instance->read = 0;
      instance->written = bytes_to_move;
   }

   available = instance->buffer_size - instance->written;
   return available;
}

unsigned ylmf_write(struct yl_memory_file *instance, const char *data, unsigned count, long long *offset) {
   unsigned written;

   assert(NULL != instance);
   
   written = 0;
   if (NULL != instance) {
      unsigned available;

      assert(NULL != instance->buffer);
      assert(instance->buffer_size >= instance->written);

      available = try_to_move(instance, count);

      written = fmin(count, available);
      copy_from_user(instance->buffer + instance->written, data, written);
      instance->written += written;
      
      *offset += written;
   }

   return written;
}

unsigned ylmf_read(struct yl_memory_file *instance, char *buffer, unsigned count, long long *offset) {
   unsigned read;

   assert(NULL != instance);
   
   read = 0;
   if (NULL != instance) {
      unsigned available;

      assert(NULL != instance->buffer);
      
      available = ylmf_get_buffer_available(instance);
      
      read = fmin(count, available);
      copy_to_user(buffer, instance->buffer + instance->read, read);
      instance->read += read;
      
      *offset += read;
   }
   
   return read;
}
