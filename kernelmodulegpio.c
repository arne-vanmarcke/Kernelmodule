#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
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

module_param_array(outputs, int, &arr_argc, 0000);
MODULE_PARM_DESC(outputs, "Set gpio's");
module_param_array(level, int, &arr_argc, 0000);
MODULE_PARM_DESC(level, "Set gpio levels");
module_param_array(togglespeed, int, &arr_argc, 0000);
MODULE_PARM_DESC(togglespeed, "Set gpio togglespeed");

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
    
    return 0;
}
static void __exit kernelmodule_exit(void)
{
    int i;

    for(i = 0; i < ARRAY_SIZE(leds); i++) {
		gpio_set_value(leds[i].gpio, 0); 
	}

    gpio_free_array(leds, ARRAY_SIZE(leds));
    printk(KERN_INFO "Exit\n");
}
module_init(kernelmodule_init);
module_exit(kernelmodule_exit);