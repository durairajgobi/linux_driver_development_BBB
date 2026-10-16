# GPIO Control

### Video :
[![Youtube Video](https://img.youtube.com/vi/wSmjAxeJr34/0.jpg)](https://www.youtube.com/watch?v=wSmjAxeJr34)

> The video was recorded on a Raspberry Pi (GPIO21 LED / GPIO20 button). The BeagleBone Black version below uses
> the same `gpiod_*` API – only the pins and GPIO numbers change.

## BeagleBone Black wiring

| Function | Pin | AM335x | Default global number (kernel 6.1 – 6.12) |
|---|---|---|---|
| LED (output) | **P9_12** | GPIO1_28 | 540 |
| Button (input, pull-up, active low) | **P9_15** | GPIO1_16 | 528 |

```
P9_12 ──[330R]──►|── P9_1 (GND)          P9_15 ──[ push button ]── P9_2 (GND)
```

## Build & run

```bash
config-pin P9_12 gpio          # make sure both pads are in GPIO mode (mode 7)
config-pin P9_15 gpio_pu       # enable the internal pull-up for the button

cd request_free                # or set_get
make

# find the global GPIO numbers on *your* kernel (see chapter 04, section 6)
sudo ../../tools/bbb_gpio_num.sh P9_12      # -> 540
sudo ../../tools/bbb_gpio_num.sh P9_15      # -> 528

sudo insmod req_free.ko                                   # defaults: led_gpio=540 button_gpio=528
sudo insmod req_free.ko led_gpio=540 button_gpio=528      # explicit
dmesg | tail
sudo rmmod req_free
```

`set_get.ko` turns the LED on at load time and prints the button state – press and hold the button
while loading to see `Button state is: 0`.

### Why `512 + 28` and not `21 + 512`?

The Raspberry Pi has a single 54-line GPIO controller whose base is 512.
The AM335x has four 32-line banks (GPIO0..GPIO3); each becomes its own `gpiochip` with its own base.
P9_12 is line 28 of bank **GPIO1**, and on 6.1 – 6.12 BeagleBoard.org kernels GPIO1 is registered first
(base 512). See [04_beaglebone_black_setup](../04_beaglebone_black_setup/readme.md#6-gpio-numbers-on-the-beaglebone-black-important).
Hard-coded numbers are fragile – that is exactly why [chapter 26](../26_dt_gpio/) moves the GPIOs into the Device Tree.
