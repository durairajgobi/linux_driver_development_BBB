#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio/consumer.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MPCoding - LDD");
MODULE_DESCRIPTION("GPIO set get Example");

/*
 * ---------------------------------------------------------------------------
 * BeagleBone Black wiring (see ../../04_beaglebone_black_setup/readme.md)
 *
 *   LED    : P9_12 (GPIO1_28) -> 330R -> LED anode, LED cathode -> GND (P9_1)
 *   BUTTON : P9_15 (GPIO1_16) -> push button -> GND (pull-up enabled, see readme)
 *
 * Linux 6.x numbers the four AM335x GPIO banks dynamically from 512.
 * On BeagleBoard.org 6.1 / 6.6 / 6.12 kernels GPIO1 registers first, so
 * GPIO1_N = 512 + N  (P9_12 = 540, P9_15 = 528).  The order has changed
 * between kernel releases, so the numbers are module parameters - look them
 * up on your board and override if needed:
 *
 *   sudo insmod <module>.ko \
 *        led_gpio=$(sudo <repo>/tools/bbb_gpio_num.sh P9_12) \
 *        button_gpio=$(sudo <repo>/tools/bbb_gpio_num.sh P9_15)
 * ---------------------------------------------------------------------------
 */
#define GPIO1_BASE   512
#define LED_GPIO     28   /* P9_12 = GPIO1_28 */
#define BUTTON_GPIO  16   /* P9_15 = GPIO1_16 */

static int led_gpio = (GPIO1_BASE + LED_GPIO);
module_param(led_gpio, int, 0444);
MODULE_PARM_DESC(led_gpio, "global GPIO number of the LED (default P9_12)");
static int button_gpio = (BUTTON_GPIO + GPIO1_BASE);
module_param(button_gpio, int, 0444);
MODULE_PARM_DESC(button_gpio, "global GPIO number of the button (default P9_15)");

static const char *device_name = "gpio_ctrl";



static struct gpio_desc *led, *button;


static int __init my_init(void) {
    int status;
	led = gpio_to_desc(led_gpio);
	if(!led){
		pr_err("%s: unable to get led_gpio (P9_12)\n", device_name);
		return -1;
	}

	button = gpio_to_desc(button_gpio);
	if(!button){
		pr_err("%s: unable to get button_gpio (P9_15)\n", device_name);
		return -1;
	}

	// Set P9_12(LED) as GPIO output
	status = gpiod_direction_output(led, 0);
	if(status){
		pr_err("%s: unable to set P9_12 as output\n", device_name);
		return -1;
	}
	
	// Set P9_15(BUTTON) as GPIO input
	status = gpiod_direction_input(button);
	if(status){
		pr_err("%s: unable to set P9_15 as input\n", device_name);
		return -1;
	}

	gpiod_set_value(led, 1); // Turn ON

	pr_info("%s: Button state is: %d\n", device_name, gpiod_get_value(button));

	return 0;
}

static void __exit my_exit(void){
    gpiod_set_value(led, 0); // Turn OFF
	pr_info("%s: Goodbye Kernel\n", device_name);
}

module_init(my_init);
module_exit(my_exit);