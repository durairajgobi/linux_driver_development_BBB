# ✅ Linux Driver Development with BeagleBone Black - Full Tutorial Series Roadmap

A BeagleBone Black (TI AM335x, ARM Cortex-A8, 32-bit) port of
[linux_driver_development_with_raspberrypi](https://github.com/durairajgobi/linux_driver_development_with_raspberrypi).
The chapter numbering, driver code and explanations follow the original series; everything that depends on
the board has been reworked for the BeagleBone Black:

| Topic | Raspberry Pi (original) | BeagleBone Black (this repo) |
|---|---|---|
| Kernel headers | `raspberrypi-kernel-headers` | `linux-headers-$(uname -r)` (BeagleBoard.org Debian 12) |
| Build | native only | native, or cross-compile: `make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- KDIR=...` |
| LED / Button | BCM GPIO21 / GPIO20 (`+512`) | **P9_12** (GPIO1_28) / **P9_15** (GPIO1_16) as module parameters |
| I2C bus | `i2c1` (`/dev/i2c-1`, pins 3/5) | **`i2c2`** (`/dev/i2c-2`, P9_19 / P9_20) |
| SPI bus | `spi0` (GPIO 8-11) | **`spi0`** (P9_17 / P9_18 / P9_21 / P9_22) |
| DT overlay target | `brcm,bcm2835`, `&gpio`, `&i2c1` | `ti,beaglebone-black`, `&gpio0..3`, `&i2c2`, `&spi0`, `&am33xx_pinmux` |
| Applying overlays | `dtoverlay` (runtime) | U-Boot: `/boot/uEnv.txt` → [`tools/bbb_overlay.sh`](tools/bbb_overlay.sh) + reboot |
| Pinmux | `brcm,pins` / `brcm,function` / `brcm,pull` | `pinctrl-single,pins = <offset value>` in the AM335x control module |

**Start with [04 – BeagleBone Black setup](04_beaglebone_black_setup/)** – it contains the pin map, wiring
and the GPIO-numbering notes every later chapter relies on.

> The YouTube videos linked in the chapters were recorded on a Raspberry Pi. The kernel concepts and APIs are
> identical; follow the readme of each chapter for the BeagleBone Black-specific commands.

### Helper scripts
* [`tools/bbb_gpio_num.sh`](tools/bbb_gpio_num.sh) – `P9_12` → global GPIO number for `gpio_to_desc()`
* [`tools/bbb_overlay.sh`](tools/bbb_overlay.sh) – enable / disable / list U-Boot overlays in `/boot/uEnv.txt`



## SECTION 0 - Introduction & Preparation

### 1. Introduction to the Series
* What the series covers
* Tools, requirements
* Recommended references (inc. Linux Device Drivers 3rd Edition)

### 2. Understanding the Linux Kernel 
* Kernel architecture
* User space vs kernel space

### 3. Linux Device Drivers Overview


## SECTION 1 - BeagleBone Black Setup & Development Environment

### [4. BeagleBone Black Setup](/04_beaglebone_black_setup/)
* Flashing BeagleBoard.org Debian 12 (armhf)
* SSH over USB (192.168.7.2)
* Installing kernel headers, native build vs cross-compiling
* P8/P9 header pin map, `config-pin`, GPIO numbering on kernel 6.x

### [5. First Kernel Module](/05_first_kernel_module/01_hello/)
* A simple hello.ko
* Makefile
* insmod, rmmod, modinfo
* [Kernel print levels (Log levels)](/05_first_kernel_module/02_log_levels/)

## SECTION 2 - Character Device Driver Foundations

### [6. Character Device Driver Basics](/06_character_device_driver/)
* Allocating major/minor numbers
* cdev_init, cdev_add
* Manual mknod vs udev auto-creation
* File operations overview

### [7. File Operations Deep Dive](/07_file_cdev_fops/)
* open, release, read, write
* copy_to_user, copy_from_user
* Using file->private_data

### [8. IOCTL Implementation](/08_ioctl/)
* Defining IOCTL commands
* Handling IOCTL in driver
* User application examples

### [9. Memory Allocation Mechanisms](/09_memory_alloc/)
* kmalloc, kzalloc, vmalloc
* GFP flags
* Slab allocator overview

### [10. Kernel Linked Lists](/10_linked_list/)
* list_head, iterating, adding, deleting
* Using linked lists in drivers

## SECTION 3 - GPIO, Interrupts, Timers

### [11. GPIO Control](/11_gpio_ctrl/)
* Request/release GPIO
* Set/get GPIO state

### [12. GPIO Interrupt Handling](/12_gpio_irq/)
* request_irq
* IRQ numbers
* Debouncing
* Top-half vs bottom-half

### [13. High Resolution Timers (hrtimer)](/13_hrtimer/)
* One-shot, periodic timers

## SECTION 4 - I2C & SPI Drivers

### [14. I2C Subsystem Basics](/14_i2c_subsystem_basics/)
* Linux I2C architecture
* Client vs adapter vs driver
* Registering an I2C driver
* Creating a device via /sys/bus/i2c/devices

### [15. BMP180 Sensor Driver (Complete Example)](/15_bmp180_i2c_sensor/)
* Read/write registers
* Calibration
* Temperature & pressure reading
* Exposing sensor values to user space

### [16. SPI Subsystem Basics](/16_spi_subsytem_basics/)
* SPI architecture
* Full duplex transfers
* SPI message & transfer structs

## SECTION 5 - Concurrency & Synchronization

### [17. Kernel Threads (kthread)](/17_kernel_threads/)
* Creating, running, stopping threads
* Thread loop best practices

### [18. Wait Queues, Poll, Fasync](/18_wait_queue_poll_fasync/)
* [Blocking/non-blocking I/O](/18_wait_queue_poll_fasync/02_wait_queue/)
* [Implementing .poll() & select()](/18_wait_queue_poll_fasync//03_poll_select/)
* [Async notification using fasync](/18_wait_queue_poll_fasync/04_fasync/)

### [19. Kernel Synchronization Techniques](/19_kernel_synchronization/)
* [Mutex](/19_kernel_synchronization/01_mutex/)
* [Spinlock](/19_kernel_synchronization/02_spinlock/)
* [Completion](/19_kernel_synchronization/03_completion/)
* [When to use which](/19_kernel_synchronization/When_to_use_which/)

### [20. Workqueues & Bottom Halves](/20_workqueue_and_bottom_halves/)
* Deferred execution
* System vs dedicated workqueues
* Comparison with tasklets

## SECTION 6 - Linux Device Model & Sysfs

### [21. Linux Device Model](/21_linux_device_module/)
* Devices, drivers, buses, classes
* Probe/remove lifecycle
* Hotplug & Uevents (plug/unplug)

### [22. Sysfs Interface](/22_sysfs/)
* Device attributes
* Creating and removing sysfs files
* Sysfs class interfaces (/sys/class/mydriver)

### [23. Debugfs for Driver Debugging](/23_debugfs/)
* Creating debugfs entries
* Live introspection of driver variables
* Differences from sysfs

## SECTION 7 - Device Tree & BeagleBone Black Overlays

### [24. Device Tree Introduction](/24_device_tree_intro/)
* Why Device Tree exists
* Structure of .dts and .dtsi
* Status, compatible, reg, gpios

### [25. Writing BeagleBone Black Device Tree Overlays](/25_device_tree_overlay/)
* Overlay structure
* Adding nodes dynamically
* Applying overlays with U-Boot (`/boot/uEnv.txt`)

### [26. Device Tree - GPIO Example](/26_dt_gpio/)
* Creating a GPIO LED/button node
* Binding to your driver
* Reading GPIO from DT

### [27. Device Tree - I2C Device](/27_dt_i2c/)
* Adding BMP180 in DT
* Custom compatible string
* Passing configuration via DT

### [28. Platform Driver](/28_platform_driver/) 
* Introduction to platform bus
* Platform_device vs platform_driver
* Useful for non-I2C/SPI devices

### [29. Pin Control & Pinmux](/29_pinmux/)
* Pin configuration in DT
* AM335x pad modes (`pinctrl-single`) on BeagleBone Black pins

### [30. OF (Open Firmware) Parsing](/30_of_parsing/)
* Using DT helper APIs
* Parsing strings, ints, gpios from DT

## SECTION 8 - Advanced Topics

### [31. Misc Device Driver](/31_misc_devices/)
* Easiest way to expose /dev/mydevice
* When to prefer over char device

### [32. Regmap API](/32_ragmap_api/)
* Excellent for I2C/SPI register-mapped chips
* Reduces boilerplate
* Cache & sync

### 33. DMA Engine Basics & DMA memcpy
* DMA vs CPU copy
* DMA mapping APIs
* Simple DMA example

### 34. Accessing Files from Kernel (Safely)
* filp_open, kernel_read, kernel_write
* Kernel warnings & limitations
* Practical example: writing logs to SD card

### 35. Memory Mapping (mmap)
* Expose kernel buffers to user space
* Example: custom framebuffer
* Page faults inside driver

### 36. Industrial I/O (IIO) Framework
* When to use IIO
* Creating an IIO device
* Reading ADC-like data

### 37. Sending Signals to User Space
* Real-time signals
* Notify app when an event occurs
* Example: IRQ → signal to application

## SECTION 9 - Serial Drivers
### 38. UART (Serial) Driver
* UART on BeagleBone Black (UART1/2/4 on P9)
* Writing a minimal UART driver
* Using termios from user space

## SECTION 10 - Final Project
### 39. Complete BeagleBone Black Kernel Driver Project
* A sophisticated final driver combining everything:
* Device Tree overlay
* Char device with IOCTL
* Sysfs interface
* GPIO + interrupt + poll
* High-resolution timer
* Optional I2C or SPI communication
* debugfs runtime variables
* User-space test application

### 40. Packaging & Publishing
* Versioning
* DKMS support
* GitHub best practices
* Share as a Linux kernel module package