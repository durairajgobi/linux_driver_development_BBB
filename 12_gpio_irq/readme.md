# GPIO Interrupt Handling

### Video :
[![Youtube Video](https://img.youtube.com/vi/hVp2e-uTYMs/0.jpg)](https://www.youtube.com/watch?v=hVp2e-uTYMs)



> The video was recorded on a Raspberry Pi. On the BeagleBone Black the code is identical apart from the pins.

## BeagleBone Black wiring

Same as [chapter 11](../11_gpio_ctrl/): LED on **P9_12** (GPIO1_28), push button from **P9_15** (GPIO1_16) to GND
with the internal pull-up enabled. A press produces a **falling edge** → `IRQF_TRIGGER_FALLING`.

```bash
config-pin P9_12 gpio
config-pin P9_15 gpio_pu

cd irq            # or top_bottom_half
make
sudo insmod irq.ko led_gpio=$(sudo ../../tools/bbb_gpio_num.sh P9_12) \
                   button_gpio=$(sudo ../../tools/bbb_gpio_num.sh P9_15)
dmesg -w          # press the button: LED toggles, "Valid button press detected"
```

Check the interrupt:

```bash
grep btn_irq_handler /proc/interrupts
#  NN:   12  4804c000.gpio  16 Edge   btn_irq_handler    <- bank GPIO1 (0x4804c000), line 16
```

### AM335x notes
* Every GPIO bank is also an interrupt controller (`gpio-omap` driver), so `gpiod_to_irq()` returns a
  Linux IRQ number mapped onto the GPIO1 bank's interrupt domain.
* The AM335x GPIO block has a hardware **debounce** (`gpiod_set_debounce()`, 31 µs steps up to ~7.9 ms).
  The software debounce timer used here works the same on every board and handles longer bounces.
