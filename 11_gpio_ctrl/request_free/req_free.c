#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MPCoding - LDD");
MODULE_DESCRIPTION("GPIO Requset Example");

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




static int __init my_init(void) {
    int status;
    status = gpio_request(led_gpio, "led_gpio");
    if(status){
        pr_err("%s: Failed to request led P9_12\n", device_name);
        return -status;
    }

    status = gpio_direction_output(led_gpio, 0);
    if(status){
        pr_err("%s: Failed to set P9_12 led direction\n", device_name);
        gpio_free(led_gpio);
        return -status;
    }

    status = gpio_request(button_gpio, "button_gpio");
    if(status){
        pr_err("%s: Failed to request button P9_15\n", device_name);
        gpio_free(led_gpio);
        return -status;
    }

    status = gpio_direction_input(button_gpio);
    if(status){
        pr_err("%s: Failed to set P9_15 button direction\n", device_name);
        gpio_free(led_gpio);
        gpio_free(button_gpio);
        return -status;
    }

    gpio_set_value(led_gpio, 1);

    pr_info("%s: Button state is: %d\n", device_name, gpio_get_value(button_gpio));

    pr_info("%s: GPIO request example loaded\n", device_name);

    return 0;
}

static void __exit my_exit(void){
    gpio_set_value(led_gpio, 0);
    gpio_free(led_gpio);
    gpio_free(button_gpio);
    pr_info("%s: Goodbye Kernel\n", device_name);
}

module_init(my_init);
module_exit(my_exit);