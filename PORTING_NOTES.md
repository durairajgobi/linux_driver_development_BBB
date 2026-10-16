# Raspberry Pi → BeagleBone Black porting notes

What changed compared with
[linux_driver_development_with_raspberrypi](https://github.com/durairajgobi/linux_driver_development_with_raspberrypi),
chapter by chapter. Chapters not listed are board-independent and only received the common Makefile change.

## All chapters
* **Makefiles**: `KDIR ?= /lib/modules/$(uname -r)/build`, `$(MAKE) -C $(KDIR) M=$(CURDIR)`.
  Native build on the board is unchanged (`make`); cross-compiling from a PC is
  `make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- KDIR=<bb-kernel tree>`.
* Every module was cross-compiled against Linux 6.12 (`omap2plus_defconfig`, 32-bit ARMv7) as a check.

## 32-bit ARM fixes (the RPi series targets 64-bit Raspberry Pi OS)
| File | Issue on the BBB | Fix |
|---|---|---|
| `10_linked_list/my_linked_list.c` | `kmalloc`/`kfree` implicit declaration (build error) – `<linux/slab.h>` isn't pulled in indirectly on ARMv7 | `#include <linux/slab.h>` |
| `09_memory_alloc/memory_alloc.c` | `virt_to_phys()` printed with `%llx`, but `phys_addr_t` is 32-bit on the AM335x | print with `%pa` |
| `07_file_cdev_fops/*/test1.c`, `08_ioctl/test1.c`, `08_ioctl/Makefile` | `ssize_t` printed with `%d`/`%ld` (wrong on one of 32/64-bit); test app always built with host `gcc` | `%zd`; `$(CROSS_COMPILE)gcc` |

## Board-specific changes
| Chapter | Raspberry Pi | BeagleBone Black |
|---|---|---|
| 04 setup | Raspberry Pi OS, `raspberrypi-kernel-headers` | BeagleBoard.org Debian 12 armhf, `linux-headers-$(uname -r)`, cross-compile, pin map, GPIO numbering |
| 06 char dev | `raspi-config` serial, `/dev/ttyS0` | UART1 on P9_24/P9_26 (`/dev/ttyS1`, 4:65) – ttyS0 is the BBB console |
| 11, 12, 18, 19, 20 GPIO | GPIO21 LED / GPIO20 button, `+512` | P9_12 (GPIO1_28) LED / P9_15 (GPIO1_16) button, `GPIO1_BASE 512`, **module parameters** `led_gpio` / `button_gpio`, `tools/bbb_gpio_num.sh` |
| 14, 15 I2C | `i2c-1`, `raspi-config` | `i2c-2` (P9_19/P9_20, enabled by default), `i2c_bus` module parameter; fixed `i2c_get_adapter()` NULL check; test app uses `%04x` for the `2-0077` sysfs name |
| 16 SPI | `spi-bcm2835`, `dtparam=spi=on` | `spi-omap2-mcspi`, SPI0 on P9_17/18/21/22 via `config-pin` |
| 19 spinlock | 4-core SMP | single Cortex-A8 – IRQ disabling is the real protection |
| 24 DT intro | `bcm2711-rpi-4-b.dts` walk-through | `am335x-boneblack.dts` walk-through (INTC 1-cell interrupts, GPIO banks, ti-sysc, ranges, pinctrl-single) |
| 25 overlays | `dtoverlay` at runtime | U-Boot overlays in `/boot/uEnv.txt` (`uboot_overlay_addr4..7`), `tools/bbb_overlay.sh`, `make install` + reboot |
| 26 DT GPIO | `<&gpio 20 0>` | `<&gpio1 28 GPIO_ACTIVE_HIGH>` + `&am33xx_pinmux` fragment |
| 27 DT I2C | `target = <&i2c1>` | `target = <&i2c2>` |
| 29 pinmux | `brcm,pins/function/pull` under `&gpio`, `alt0` state | `pinctrl-single,pins = <AM33XX_PADCONF(...)>` under `&am33xx_pinmux`, `uart` state on P9_24/P9_26, pad-register debugfs |
| 30 OF parsing | `reset-gpios = <&gpio 27 1>` | `reset-gpios = <&gpio1 17 GPIO_ACTIVE_LOW>` (P9_23) + pinmux |
| 32 regmap | `&i2c1`, `&spi0` + `__overrides__` | `&i2c2` @0x68; SPI0 overlay muxes pads + enables `&spi0` (no `__overrides__` in U-Boot) |

## Overlay build
Overlays are preprocessed with `cpp -I $(KDIR)/include` before `dtc -@`, so they can use
`<dt-bindings/gpio/gpio.h>` and `<dt-bindings/pinctrl/am33xx.h>`. All overlays were checked with
`fdtoverlay` against a mainline `am335x-boneblack.dtb`.

Overlays that set their own pinmux (26, 29, 30, 32-spi) need **cape-universal disabled**;
`make install` passes `--no-universal` to `tools/bbb_overlay.sh` for those chapters.

## Not changed
* YouTube links (recorded on a Raspberry Pi – APIs are identical).
* `16_spi_subsytem_basics/ESP32_SPI_Slave` (wire it to SPI0 as described in the readme).
* Diagrams that are board-independent.
