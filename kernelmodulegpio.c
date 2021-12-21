#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/stat.h>
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <asm/uaccess.h>

#include "query_ioctl.h"

#define FIRST_MINOR 0
#define MINOR_CNT 1

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Arne Vanmarcke");

static int outputs[2] = { -1, -1 };
static int level[2] = { -1, -1 };
static int togglespeed[2] = { -1, -1 };
static int arr_argc = 0;
char led_name[20]="Led 24";
const char *ptrLedName=&led_name[0];

struct gpio leds[(sizeof outputs / sizeof (int))];
struct timer_list blink_timer;
struct timer_list blink_timer_2;
static dev_t dev;
static struct cdev c_dev;
static struct class *cl;
static int status = 1, dignity = 3, ego = 5;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35))
static int my_ioctl(struct inode *i, struct file *f, unsigned int cmd, unsigned long arg)
#else
static long my_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
#endif
{
    query_arg_t q;
    switch (cmd){
        case QUERY_GET_VARIABLES:
            q.gpio = status;
            q.level = dignity;
            q.togglespeed = ego;
            printk(KERN_INFO "GET: gpio: %d, level: %d, togglespeed: %d\n", q.gpio, q.level, q.togglespeed);
            if (copy_to_user((query_arg_t *)arg, &q, sizeof(query_arg_t)))
            {
                return -EACCES;
            }
            break;
        case QUERY_CLR_VARIABLES:
            status = 0;
            dignity = 0;
            ego = 0;
            break;
        case QUERY_SET_VARIABLES:
            if (copy_from_user(&q, (query_arg_t *)arg, sizeof(query_arg_t)))
            {
                return -EACCES;
            }
            status = q.gpio;
            dignity = q.level;
            ego = q.togglespeed;
            printk(KERN_INFO "SET: gpio: %d, level: %d, togglespeed: %d\n", q.gpio, q.level, q.togglespeed);
            break;
        default:
            return -EINVAL;
    }
    return -EINVAL;
}

static int my_open(struct inode *i, struct file *f)
{
    return 0;
}
static int my_close(struct inode *i, struct file *f)
{
    return 0;
}

static struct file_operations query_fops =
{
    .owner = THIS_MODULE,
    .open = my_open,
    .release = my_close,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35))
    .ioctl = my_ioctl
#else
    .unlocked_ioctl = my_ioctl
#endif
};


module_param_array(outputs, int, &arr_argc, 0000);
MODULE_PARM_DESC(outputs, "Set gpio's");
module_param_array(level, int, &arr_argc, 0000);
MODULE_PARM_DESC(level, "Set gpio levels");
module_param_array(togglespeed, int, &arr_argc, 0000);
MODULE_PARM_DESC(togglespeed, "Set gpio togglespeed");

/*static void blink_timer_func(struct timer_list* t)
{
	printk(KERN_INFO "%s\n", __func__);

	gpio_set_value(outputs[0], level[0]);
	level[0]^=1;
	blink_timer.expires = jiffies + (togglespeed[0]*HZ);
	add_timer(&blink_timer);
}

static void blink_timer_2_func(struct timer_list* t)
{
	printk(KERN_INFO "%s\n", __func__);

	gpio_set_value(outputs[1], level[1]);
	level[1]^=1;
	blink_timer_2.expires = jiffies + (togglespeed[1]*HZ);
	add_timer(&blink_timer_2);
}*/

static int __init kernelmodule_init(void){
    //int i;
    int ret;
    struct device *dev_ret;

    /*for (i = 0; i < (sizeof outputs / sizeof (int)); i++)
    {
        printk(KERN_INFO "outputs[%d] = %d\nlevels[%d] = %d\ntogglespeed[%d]= %d\n", i, outputs[i], i, level[i], i, togglespeed[i]);
        leds[i].gpio=outputs[i];
        if(level[i])
            leds[i].flags=GPIOF_OUT_INIT_HIGH;
        else
            leds[i].flags=GPIOF_OUT_INIT_LOW;
        leds[i].label="Led";
    }

    ret = gpio_request_array(leds, ARRAY_SIZE(leds));

	if (ret) {
		printk(KERN_ERR "Unable to request GPIOs: %d\n", ret);
	}

    if(togglespeed[0]!=0){
        timer_setup(&blink_timer, blink_timer_func, 0);
        blink_timer.function = blink_timer_func;
        //blink_timer.data = 1L;							// initially turn LED on
        blink_timer.expires = jiffies + (togglespeed[0]*HZ); 		// 1 sec.
        add_timer(&blink_timer);
    }
    if(togglespeed[1]!=0){
        timer_setup(&blink_timer_2, blink_timer_2_func, 0);
        blink_timer_2.function = blink_timer_2_func;
        //blink_timer.data = 1L;							// initially turn LED on
        blink_timer_2.expires = jiffies + (togglespeed[1]*HZ); 		// 1 sec.
        add_timer(&blink_timer_2);
    }*/

    if ((ret = alloc_chrdev_region(&dev, FIRST_MINOR, MINOR_CNT, "query_ioctl")) < 0)
    {
        return ret;
    }
 
    cdev_init(&c_dev, &query_fops);
 
    if ((ret = cdev_add(&c_dev, dev, MINOR_CNT)) < 0)
    {
        return ret;
    }
     
    if (IS_ERR(cl = class_create(THIS_MODULE, "char")))
    {
        cdev_del(&c_dev);
        unregister_chrdev_region(dev, MINOR_CNT);
        return PTR_ERR(cl);
    }
    if (IS_ERR(dev_ret = device_create(cl, NULL, dev, NULL, "query")))
    {
        class_destroy(cl);
        cdev_del(&c_dev);
        unregister_chrdev_region(dev, MINOR_CNT);
        return PTR_ERR(dev_ret);
    }

    
    return 0;
}
static void __exit kernelmodule_exit(void)
{
    //int i;
    
    device_destroy(cl, dev);
    class_destroy(cl);
    cdev_del(&c_dev);
    unregister_chrdev_region(dev, MINOR_CNT);

    /*del_timer_sync(&blink_timer);
    del_timer_sync(&blink_timer_2);

    for(i = 0; i < ARRAY_SIZE(leds); i++) {
		gpio_set_value(leds[i].gpio, 0); 
	}

    gpio_free_array(leds, ARRAY_SIZE(leds));*/
    printk(KERN_INFO "Exit\n");
}
module_init(kernelmodule_init);
module_exit(kernelmodule_exit);