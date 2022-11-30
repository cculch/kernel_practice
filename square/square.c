#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kdev_t.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>

MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("cculch");
MODULE_DESCRIPTION("kernel practice : square");
MODULE_VERSION("0.1");

#define DEV_NAME "square"

#define MAX_LENGTH 92

static dev_t squ_dev = 0;
static struct cdev *squ_cdev;
static struct class *squ_class;
static DEFINE_MUTEX(squ_mutex);

static long long square(long long num)
{
	return num * num;
}

static int squ_open(struct inode *inode, struct file *file)
{
	if(!mutex_trylock(&squ_mutex))
	{
		printk(KERN_ALERT "mutex_trylock() failed!");
		return -EBUSY;
	}

	return 0;
}

static int squ_release(struct inode *inode, struct file *file)
{
	mutex_unlock(&squ_mutex);
	return 0;
}

static ssize_t squ_read(struct file *file, char *buf, size_t size, loff_t *offset)
{
	return (ssize_t) square(*offset);
}

static ssize_t squ_write(struct file *file, const char *buf, size_t size, loff_t *offset)
{
	return 1;
}

static loff_t squ_device_lseek(struct file *file, loff_t offset, int orig)
{
	loff_t new_pos = 0;

	switch(orig)
	{
		case 0: // SEEK_SET
			new_pos = offset;
			break;
		case 1: // SEEK_CUR
			new_pos = file->f_pos + offset;
			break;
		case 2: // SEEK_END
			new_pos = MAX_LENGTH - offset;
			break;
	}

	if(new_pos > MAX_LENGTH)
		new_pos = MAX_LENGTH;
	if(new_pos < 0)
		new_pos = 0;
	file->f_pos = new_pos;
	
	return new_pos;
}

const struct file_operations squ_fops = 
{
	.owner = THIS_MODULE,
	.read = squ_read,
	.write = squ_write,
	.open = squ_open,
	.release = squ_release,
	.llseek = squ_device_lseek,
};

/*
 * "__init" is defined in /include/linux/init.h
 * 		#define __init	__attribute__ ((__section__(".init.text"))) __cold
 * 			section ("SECTION-NAME") : specify the data or function to specific section
 * 		this function will be released after initialization
*/

 static int __init init_squ_dev(void) 
{
	int ret = 0;

	mutex_init( &squ_mutex);

	// register square device
	ret = alloc_chrdev_region(&squ_dev, 0, 1, DEV_NAME);
	if( ret < 0 )
	{
		printk( KERN_ALERT "alloc_chrdev_region() failed!, ret = %d", ret );
		return ret;
	}

	// cdev_alloc() allocates and return a cdev struct, or NULL on failure
	squ_cdev = cdev_alloc();
	if( squ_cdev == NULL )
	{
		printk(KERN_ALERT "cdev_alloc() failed!");
		ret = -1;
		goto failed_cdev;
	}
	squ_cdev->ops = &squ_fops;

	// cdev_add() adds the device (struct cdev) to the system, making it live immediately
	ret = cdev_add(squ_cdev, squ_dev, 1);
	if( ret < 0 )
	{
		printk(KERN_ALERT "cdev_add() failed!");
		ret = -2;
		goto failed_cdev;
	}

	// class_create() is a Macro used to create and initialize a class and add the class into kernel
	squ_class = class_create(THIS_MODULE, DEV_NAME);
	if(!squ_class)
	{
		printk(KERN_ALERT "class_create() failed!");
		ret = -3;
		goto failed_class_create;
	}

	// device_create() creates device file under /dev
	if(!device_create(squ_class, NULL, squ_dev, NULL, DEV_NAME))
	{
		printk(KERN_ALERT "device_create() failed!");
		ret = -4;
		goto failed_device_create;
	}
	
	return ret;

failed_device_create:
	class_destroy(squ_class);
failed_class_create:
	cdev_del(squ_cdev);
failed_cdev:
	unregister_chrdev_region(squ_dev, 1);

	return ret;
}

static void __exit exit_squ_dev(void)
{
	mutex_destroy(&squ_mutex);
	device_destroy(squ_class, squ_dev);
	class_destroy(squ_class);
	cdev_del(squ_cdev);
	unregister_chrdev_region(squ_dev, 1);
}


module_init(init_squ_dev);
module_exit(exit_squ_dev);















