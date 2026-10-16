# BeagleBone Black Setup & Development Environment

This chapter replaces the Raspberry Pi setup chapter of the original series.
Everything after this chapter assumes the board is prepared as described here.

| | Raspberry Pi 4 (original series) | BeagleBone Black (this repo) |
|---|---|---|
| SoC | Broadcom BCM2711, 4x Cortex-A72, 64-bit | TI **AM335x** (AM3358), 1x Cortex-A8 @ 1 GHz, **32-bit ARMv7** |
| RAM | 1–8 GB | 512 MB DDR3 |
| OS image | Raspberry Pi OS (64-bit) | BeagleBoard.org **Debian 12 (Bookworm) IoT / minimal, armhf** |
| Kernel headers | `raspberrypi-kernel-headers` | `linux-headers-$(uname -r)` |
| Boot config | `/boot/firmware/config.txt` | **`/boot/uEnv.txt`** (U-Boot) |
| DT overlays | `dtoverlay` (runtime) | `uboot_overlay_addrN=` in `uEnv.txt` (applied at boot) – see [chapter 25](../25_device_tree_overlay/) |
| Pin muxing from the shell | `raspi-gpio` / `pinctrl` | `config-pin` (needs cape-universal) |
| Header | 40-pin | **P8 + P9, 2 x 46 pins** |
| I/O voltage | 3.3 V | **3.3 V – NOT 5 V tolerant. ADC pins max 1.8 V** |

---

## 1. Flash the image

1. Download the latest **AM335x Debian 12 IoT** (or *minimal*) image for *BeagleBone Black* from
   <https://www.beagleboard.org/distros>.
2. Flash it to a micro-SD card (≥ 8 GB) with balenaEtcher, or:
   ```bash
   xzcat am335x-debian-12.*-armhf-*.img.xz | sudo dd of=/dev/sdX bs=4M status=progress conv=fsync
   ```
3. Insert the card, **hold the BOOT button (S2, next to the SD slot)**, apply power and release it once
   the user LEDs start blinking. Without the button the board boots the old eMMC image.
4. Optional: flash the SD image to eMMC later (see the flasher notes in `/boot/uEnv.txt`).

## 2. Log in

Over the mini-USB cable the BBB shows up as a USB network adapter:

```bash
ssh debian@192.168.7.2      # Linux / macOS host  (Windows: 192.168.6.2)
# default password: temppwd   -> change it with: passwd
```

Ethernet works too (`ssh debian@beaglebone.local`). Keep a **3.3 V USB-serial cable on the J1
header (115200 8N1)** handy – it is the only way to see what happened when an overlay breaks the boot.

Put the board on the internet (Ethernet or host sharing), then:

```bash
sudo apt update && sudo apt upgrade -y
```

## 3. Install kernel headers & build tools (native build)

```bash
uname -r                                     # e.g. 6.12.x-bone.. or 6.6.x-ti-..
sudo apt install -y build-essential device-tree-compiler gpiod i2c-tools
sudo apt install -y linux-headers-$(uname -r)
ls /lib/modules/$(uname -r)/build            # must exist
sudo reboot                                  # if apt upgraded the kernel
```

> The BeagleBoard.org repository ships a matching `linux-headers-*` package for its kernels.
> If `apt` can't find yours, move to a current kernel first (`apt-cache search bbb.io-kernel`
> lists the kernel meta-packages), reboot, and install the headers again.

Every chapter's `Makefile` then builds natively:

```bash
make
sudo insmod hello.ko
dmesg | tail
```

> The single Cortex-A8 needs a few seconds per small module – fine for this series.

## 4. (Optional) Cross-compile on a PC – much faster

```bash
sudo apt install -y gcc-arm-linux-gnueabihf make bc bison flex libssl-dev libelf-dev device-tree-compiler
```

You need a kernel tree **configured exactly like the board's kernel**:

```bash
git clone https://github.com/beagleboard/linux.git bb-kernel     # check out the tag matching `uname -r`
cd bb-kernel
scp debian@192.168.7.2:/boot/config-<uname -r> .config
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- olddefconfig
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -j$(nproc)          # or at least modules_prepare
```

Then build any chapter on the PC and copy the result over:

```bash
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- KDIR=~/bb-kernel
scp *.ko debian@192.168.7.2:~
```

> `insmod` refuses a module whose *vermagic* differs from the running kernel
> ("Invalid module format"). Compare `modinfo x.ko | grep vermagic` with `uname -r`.

## 5. Header pins used in this series

Pin naming: **P9_12** = header P9, pin 12. Each pin has a *pad* register in the AM335x control
module (`0x44E10800 + offset`) with 8 *modes*; **mode 7 is always GPIO**.

| Signal | Header pin | AM335x signal | Pad offset (from 0x800) | Chapters |
|---|---|---|---|---|
| LED (output) | **P9_12** | GPIO1_28 | `0x078` gpmc_ben1 | 11, 12, 20, 26 |
| Push button (input, pull-up) | **P9_15** | GPIO1_16 | `0x040` gpmc_a0 | 11, 12, 18, 19, 20, 26 |
| Reset line demo | P9_23 | GPIO1_17 | `0x044` gpmc_a1 | 30, 32 |
| IRQ line demo | P9_27 | GPIO3_19 | `0x1A4` mcasp0_fsr | 32 |
| I2C2 SCL | **P9_19** | I2C2_SCL (mode 3) | `0x17C` uart1_rtsn | 14, 15, 27, 32 |
| I2C2 SDA | **P9_20** | I2C2_SDA (mode 3) | `0x178` uart1_ctsn | 14, 15, 27, 32 |
| SPI0 SCLK | **P9_22** | spi0_sclk (mode 0) | `0x150` | 16, 32 |
| SPI0 D0 (MISO) | **P9_21** | spi0_d0 (mode 0) | `0x154` | 16, 32 |
| SPI0 D1 (MOSI) | **P9_18** | spi0_d1 (mode 0) | `0x158` | 16, 32 |
| SPI0 CS0 | **P9_17** | spi0_cs0 (mode 0) | `0x15C` | 16, 32 |
| UART1 TXD | P9_24 | uart1_txd / GPIO0_15 | `0x184` | 29 |
| UART1 RXD | P9_26 | uart1_rxd / GPIO0_14 | `0x180` | 29 |
| 3.3 V | P9_3, P9_4 | | | |
| GND | P9_1, P9_2 | | | |

Pins to **avoid**: P8_3–P8_6 and P8_20–P8_25 (eMMC), P8_27–P8_46 (HDMI, unless HDMI is disabled in
`uEnv.txt`). P9_19/P9_20 are I2C2, already enabled at 100 kHz in the base device tree (cape EEPROM
bus) – that is why the I2C chapters use them.

### Wiring

```
                 BeagleBone Black – P9 header
          ┌──────────────────────────────────────────┐
  GND  ── │ P9_1  P9_2  ── GND                        │
  3V3  ── │ P9_3  P9_4  ── 3V3                        │
  LED  ── │ P9_12 ──[330R]──►|── GND                  │
  BTN  ── │ P9_15 ──[ push button ]── GND  (pull-up)  │
  I2C  ── │ P9_19 SCL   P9_20 SDA   ── BMP180 / MPU   │
  SPI  ── │ P9_17 CS0   P9_18 MOSI                    │
          │ P9_21 MISO  P9_22 SCLK                    │
          └──────────────────────────────────────────┘
```

### Configure pads from the shell (chapters 11 – 23)

Until we write our own pinmux in [chapter 29](../29_pinmux/), the stock image's **cape-universal**
overlay (`enable_uboot_cape_universal=1` in `/boot/uEnv.txt`, on by default) lets `config-pin`
change every pad at run time:

```bash
config-pin P9_12 gpio            # LED (direction is set by the driver)
config-pin P9_15 gpio_pu         # button with internal pull-up
config-pin -q P9_15              # query the current mode
# chapter 16 (SPI0):
config-pin P9_17 spi_cs; config-pin P9_18 spi; config-pin P9_21 spi; config-pin P9_22 spi_sclk
```

> If `config-pin` says the pin has no *pinmux helper*, cape-universal isn't loaded – check
> `/boot/uEnv.txt` and `sudo beagle-version`.

## 6. GPIO numbers on the BeagleBone Black (important!)

The original series uses `21 + 512` because the RPi's single GPIO chip starts at 512.
The AM335x has **four GPIO banks of 32 lines** (`GPIOn_m`, legacy number `n*32 + m`). Since kernel 6.x
each bank gets a dynamically allocated base. On BeagleBoard.org 6.1 – 6.12 kernels:

| Bank | Registers | Global numbers (6.1 – 6.12) | Legacy (4.x) |
|---|---|---|---|
| GPIO1 | `0x4804C000` | 512 – 543 | 32 – 63 |
| GPIO2 | `0x481AC000` | 544 – 575 | 64 – 95 |
| GPIO3 | `0x481AE000` | 576 – 607 | 96 – 127 |
| GPIO0 | `0x44E07000` | 608 – 639 | 0 – 31 |

So **P9_12 = GPIO1_28 = 540** and **P9_15 = GPIO1_16 = 528** – the module defaults in chapters
11 – 20. The bank order has changed between kernel releases (6.15 reshuffled it again), so check on
your board – the BeagleBoard.org device tree names every line after its header pin:

```bash
sudo grep -E "\((P9_12|P9_15A?) " /sys/kernel/debug/gpio
#  gpio-528 (P9_15A              |...) in  hi      <- mainline names shared pins P9_15A / P9_15B
#  gpio-540 (P9_12               |...) out lo
sudo ../tools/bbb_gpio_num.sh P9_12          # helper in this repo -> 540
```

and override the defaults if they differ:

```bash
sudo insmod set_get.ko led_gpio=540 button_gpio=528
```

From [chapter 26](../26_dt_gpio/) onwards the GPIOs come from the Device Tree and these numbers disappear.

## 7. Useful commands

```bash
uname -r                         # kernel version
sudo beagle-version              # board, image, U-Boot overlays that were applied
cat /boot/uEnv.txt               # boot configuration / overlays
dmesg -w                         # follow the kernel log (printk output)
ls /proc/device-tree/            # live device tree
grep CONFIG_HZ= /boot/config-$(uname -r)
cp -r old_dir new_dir            # copy a chapter to start a new experiment
```

## 8. VS Code IntelliSense

`.vscode/c_cpp_properties.json` points at `/usr/src/linux-headers-<version>/` – edit the version
to match `uname -r` on the board (or your `KDIR` when cross-compiling).
