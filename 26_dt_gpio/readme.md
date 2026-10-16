# Device Tree - GPIO Example

## Video :-

[![Youtube Video](https://img.youtube.com/vi/Klkup3wPgI8/0.jpg)](https://www.youtube.com/watch?v=Klkup3wPgI8)

In the [previous video](../25_device_tree_overlay/), we learned how to write, compile, and load Device Tree Overlays, and how to read different DT property types from a platform driver using the `device_property_read_*` API family.

In this video, we take a practical step forward,     
we define `GPIO pins` directly in the **Device Tree**. We will create a DT overlay that describes an LED and a button node, bind it to a platform driver, and use the GPIO descriptor API (`gpiod_*`) to request and control those GPIOs from within the driver. This is the recommended, modern approach for GPIO handling in Linux device drivers.


### Why Define GPIOs in the Device Tree?

Hardcoding GPIO numbers in driver source code is fragile,    
GPIO numbering can differ between boards and kernel versions.   
The Device Tree solves this by describing hardware connections declaratively, separate from driver logic.    
The driver simply asks for a named GPIO property, and the kernel resolves the actual pin.

### GPIO Descriptor API (`gpiod_*`)

The modern approach in Linux is the GPIO descriptor API, which replaces the legacy `gpio_request()` / `gpio_direction_output()` style. With the descriptor API:

* GPIOs are requested by name (matching the DT property name, minus the -gpios suffix)
* The descriptor carries direction, active-low polarity, and other metadata
* It is safer, more portable, and better integrated with the Device Tree


## BeagleBone Black version

> The video uses the Raspberry Pi (`<&gpio 20 0>`). On the BeagleBone Black each of the four GPIO banks is its own
> controller, so a GPIO is written as **`<&gpioBANK LINE FLAGS>`**.

### Wiring

| Function | Pin | DT specifier |
|---|---|---|
| LED → 330R → GND | **P9_12** | `<&gpio1 28 GPIO_ACTIVE_HIGH>` |
| Button → GND (internal pull-up) | **P9_15** | `<&gpio1 16 GPIO_ACTIVE_LOW>` |

### The overlay (`dts/my_gpio_overlay.dts`)

```dts
#include <dt-bindings/gpio/gpio.h>
#include <dt-bindings/pinctrl/am33xx.h>

/ {
    compatible = "ti,am335x-bone-black", "ti,am335x-bone", "ti,am33xx";

    fragment@0 {                                    /* pad muxing: mode 7 = GPIO */
        target = <&am33xx_pinmux>;
        __overlay__ {
            mydevice_pins: mydevice-pins {
                pinctrl-single,pins = <
                    AM33XX_PADCONF(AM335X_PIN_GPMC_BEN1, PIN_OUTPUT,       MUX_MODE7) /* P9_12 */
                    AM33XX_PADCONF(AM335X_PIN_GPMC_A0,   PIN_INPUT_PULLUP, MUX_MODE7) /* P9_15 */
                >;
            };
        };
    };

    fragment@1 {
        target-path = "/";
        __overlay__ {
            mydevice: mydevice@0 {
                compatible = "testdevice,mydev";
                pinctrl-names = "default";
                pinctrl-0 = <&mydevice_pins>;
                led-gpios    = <&gpio1 28 GPIO_ACTIVE_HIGH>;
                button-gpios = <&gpio1 16 GPIO_ACTIVE_LOW>;
            };
        };
    };
};
```

Compared with the Raspberry Pi overlay:

* **No more GPIO numbers** – `devm_gpiod_get(dev, "led", ...)` finds `led-gpios`, whatever the kernel's
  global numbering is (512-based, reshuffled, …). This is the fix for the fragile numbers in chapters 11 – 20.
* **`GPIO_ACTIVE_LOW`** on the button: `gpiod_get_value()` returns **1 when pressed**, the inversion is handled by gpiolib.
* **Explicit pinmux (fragment@0)**: on the RPi the pad function was implied by the GPIO controller; on the AM335x the
  pad must be put into *mode 7* by the separate pin controller. The driver doesn't change at all – the `"default"`
  pinctrl state is applied by the driver core before `probe()` (details in [chapter 29](../29_pinmux/)).
* The `.dts` is run through `cpp` (see the `Makefile`) so the `#include`s and macros work.

### Build & run

```bash
make                    # gpio_dt_driver.ko + dts/my_gpio_overlay.dtbo
make install            # -> /lib/firmware + /boot/uEnv.txt (uboot_overlay_addr4..7),
                        #    also disables cape-universal so the pads are free
sudo reboot
make load               # insmod (only if /proc/device-tree has mydevice@0)
dmesg | tail
make unload
make uninstall && sudo reboot   # to remove the overlay
```

Inspect what the kernel resolved:

```bash
ls /proc/device-tree/mydevice@0/
sudo cat /sys/kernel/debug/gpio | grep -A0 -E "P9_12|P9_15"
#  gpio-540 (P9_12               |led                 ) out lo
#  gpio-528 (P9_15A              |button              ) in  hi ACTIVE LOW
```
