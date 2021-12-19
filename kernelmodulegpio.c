#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/stat.h>

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

module_param_array(outputs, int, &arr_argc, 0000);
MODULE_PARM_DESC(outputs, "Set gpio's");
module_param_array(level, int, &arr_argc, 0000);
MODULE_PARM_DESC(level, "Set gpio levels");
module_param_array(togglespeed, int, &arr_argc, 0000);
MODULE_PARM_DESC(togglespeed, "Set gpio togglespeed");

static void blink_timer_func(struct timer_list* t)
{
	printk(KERN_INFO "%s\n", __func__);

	gpio_set_value(outputs[0], level[0]);
	level[0]^=1;
	
	/* schedule next execution */
	//blink_timer.data = !data;						// makes the LED toggle 
	blink_timer.expires = jiffies + (togglespeed[0]*HZ); 		// 1 sec.
	add_timer(&blink_timer);
}

static void blink_timer_2_func(struct timer_list* t)
{
	printk(KERN_INFO "%s\n", __func__);

	gpio_set_value(outputs[1], level[1]);
	level[1]^=1;
	
	/* schedule next execution */
	//blink_timer.data = !data;						// makes the LED toggle 
	blink_timer_2.expires = jiffies + (togglespeed[1]*HZ); 		// 1 sec.
	add_timer(&blink_timer_2);
}

static int __init kernelmodule_init(void){
    int i;
    int ret;
    printk(KERN_INFO "Module werkt");
    for (i = 0; i < (sizeof outputs / sizeof (int)); i++)
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
    }

    // for (i = 0; i < (sizeof outputs / sizeof (int)); i++)
    // {
    //     if(togglespeed[i]!=0){
    //         index=i;
    //         timer_setup(&blink_timer[i], blink_timer_func, 0);
    //         blink_timer[i].function = blink_timer_func;
    //         //blink_timer.data = 1L;							// initially turn LED on
    //         blink_timer[i].expires = jiffies + (togglespeed[i]*HZ); 		// 1 sec.
    //         add_timer(&blink_timer[i]);
    //     }
    // }

    
    return 0;
}
static void __exit kernelmodule_exit(void)
{
    int i;
    
    del_timer_sync(&blink_timer);
    del_timer_sync(&blink_timer_2);

    for(i = 0; i < ARRAY_SIZE(leds); i++) {
		gpio_set_value(leds[i].gpio, 0); 
	}

    gpio_free_array(leds, ARRAY_SIZE(leds));
    printk(KERN_INFO "Exit\n");
}
module_init(kernelmodule_init);
module_exit(kernelmodule_exit);