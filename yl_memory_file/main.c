#include "yl_memory_file.h"

#include <linux/init.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>

static const int SUCCESS = 0;
static const char *DEVICE_NAME = "ylmfdev";	/* Dev name as it appears in /proc/devices   */

static dev_t first;			// Global variable for the first device number 
static struct cdev c_dev;	// Global variable for the character device structure
static struct class *cl;	// Global variable for the device class

static struct yl_memory_file *memory_file = NULL;

static const int buffer_size = 4*1024*1024;

static int my_open(struct inode *i, struct file *f)
{
  printk(KERN_INFO "Driver: open()\n");
  
  /*
  if (NULL != memory_file) {
	  return -EBUSY;
  }
  else {
	  memory_file = ylmf_create_with_buffer_size(buffer_size);
  }
  */
  return SUCCESS;
}

static int my_close(struct inode *i, struct file *f)
{
  printk(KERN_INFO "Driver: close()\n");
  
  /*
  ylmf_destroy(&memory_file);
  */
  
  return SUCCESS;
}

static ssize_t my_read(struct file *f, char __user *buf, size_t
  len, loff_t *off)
{
   ssize_t read_count;
	
   printk(KERN_INFO "Driver: read()\n");
  
   read_count = ylmf_read(memory_file, buf, len, off);
  
   return read_count;
}

static ssize_t my_write(struct file *f, const char __user *buf, size_t len, loff_t *off)
{
	ssize_t written_count;
	
   printk(KERN_INFO "Driver: write()\n");
   
   written_count = ylmf_write(memory_file, buf, len, off);
  
   return written_count;
}

static struct file_operations pugs_fops =
{
  .owner = THIS_MODULE,
  .open = my_open,
  .release = my_close,
  .read = my_read,
  .write = my_write
};
  
static int __init ylvfs_driver_init(void)
{
	int major;
	
   printk(KERN_INFO "Registering virtual file system\n");
   
   if (alloc_chrdev_region(&first, 0, 1, "ylmf_device") < 0)
   {
      return -1;
   }
   if ((cl = class_create(THIS_MODULE, "chardrv")) == NULL)
   {
      unregister_chrdev_region(first, 1);
      return -1;
   }
   if (device_create(cl, NULL, first, NULL, DEVICE_NAME) == NULL)
   {
      class_destroy(cl);
      unregister_chrdev_region(first, 1);
      return -1;
   }
   cdev_init(&c_dev, &pugs_fops);
   if (cdev_add(&c_dev, first, 1) == -1)
   {
      device_destroy(cl, first);
      class_destroy(cl);
      unregister_chrdev_region(first, 1);
      return -1;
   }

	memory_file = ylmf_create_with_buffer_size(buffer_size);
	if (NULL == memory_file) {
		return -1;
	}
	
   printk(KERN_INFO "Registered virtual file system\n");
   
   major = first;
   printk(KERN_INFO "I was assigned major number %d. To talk to\n", major);
	printk(KERN_INFO "the driver, create a dev file with\n");
	printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, major);
	printk(KERN_INFO "Try various minor numbers. Try to cat and echo to\n");
	printk(KERN_INFO "the device file.\n");
	printk(KERN_INFO "Remove the device file and module when done.\n");
	
   return 0;
}

static void __exit ylvfs_driver_exit(void)
{
   printk(KERN_INFO "Unregistering virtual file system\n");

   ylmf_destroy(&memory_file);

   cdev_del(&c_dev);
   device_destroy(cl, first);
   class_destroy(cl);
   unregister_chrdev_region(first, 1);
   
   printk(KERN_INFO "Unregistered virtual file system\n");
   
   return;
}

module_init(ylvfs_driver_init);
module_exit(ylvfs_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yuriy Levytskyy");
MODULE_DESCRIPTION("Virtual File System Driver");
