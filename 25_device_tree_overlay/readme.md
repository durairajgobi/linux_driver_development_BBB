# Writing BeagleBone Black Device Tree Overlays

> The video was recorded on a Raspberry Pi. The overlay syntax and the driver are identical on the BeagleBone Black;
> **how the overlay gets applied is different** (U-Boot at boot time instead of `dtoverlay` at runtime) – see Step 3.

## Video :-

[![Youtube Video](https://img.youtube.com/vi/kYs5COiMf6M/0.jpg)](https://www.youtube.com/watch?v=kYs5COiMf6M)

## Overview

In the [previous video](../24_device_tree_intro/readme.md), we got a high-level introduction to what the **Device Tree** is and how it describes hardware to the Linux kernel.     
Now it is time to go deeper and get hands-on.

In this vodeo, we will write a **Device Tree Overlay**,   
A small patch that adds a new device node to the running Device Tree without modifying the board's base `.dtb` file.    
We will cover the overlay file structure, compile it using `dtc`, apply it on the BeagleBone Black through U-Boot, and then access all of its properties from inside a platform driver module.

---------------------

## What Is a Device Tree Overlay?

The base **Device Tree Blob** (`.dtb`) for BeagleBone, Raspberry Pi, NXP, Radxa, etc.. is compiled at build time and describes all the hardware that ships with the board.     
A **Device Tree Overlay** (`.dtbo`) is an additional, independently compiled fragment that gets merged into the live Device Tree at runtime.     
This makes overlays the standard way to:
* Describe custom hardware connected to the board (sensors, displays, HATs)
* Enable or disable peripherals without rebuilding the kernel
* Inject custom properties that a loadable driver module can read

>> **Note:** On the BeagleBone Black (BeagleBoard.org Debian), overlays are `.dtbo` files in `/lib/firmware/` that **U-Boot** merges into the base `.dtb` before starting the kernel. They are listed in `/boot/uEnv.txt` as `uboot_overlay_addr0..7=`. The stock BeagleBoard.org kernels have no runtime `dtoverlay` equivalent, so applying an overlay = edit `uEnv.txt` + reboot.

( We talked about these in the [last video](../24_device_tree_intro/) in detail. )

-------

### Step 1: The Device Tree Overlay File

#### 1.1 Expanded Overlay : Multiple Property Types

Below is the overlay used in this tutorial. It is intentionally designed to demonstrate some of the common DT property data types defined in the Device Tree Specification.

`testoverlay.dts`:
```c
/dts-v1/;
/plugin/;

/ {
    compatible = "ti,am335x-bone-black", "ti,am335x-bone", "ti,am33xx";

    fragment@0 {
        target-path = "/";
        __overlay__ {
            my_device {
                compatible = "testdevice,mydev";
                status = "okay";

                /* String property */
                label = "TestDevice";

                /* 32-bit unsigned integer */
                my_value = <12>;

                /* 8-bit unsigned integer */
                my_small_val = /bits/ 8 <0x42>;

                /* Boolean property — presence means true */
                my_flag;

                /* Array of 32-bit integers */
                my_array = <10 20 30 40>;

                /* List of strings */
                my_stringlist = "alpha", "beta", "gamma";
            };
        };
    };
};
```

#### 1.2 DT Property Types Summary

Property | DT | Syntax | Kernel API C Type |
---------|----|--------|--------------------|
**label**  | `"string"` | `device_property_read_string()` | `const char *`
**my_value** | `<12>` | `device_property_read_u32()` | `u32`
**my_small_val** | `/bits/ 8 <0x42>` | `device_property_read_u8()` | `u8`
**my_flag** | (no value) | `device_property_present()` | boolean check
**my_array** | `<10 20 30 40>` | `device_property_read_u32_array()` | `u32[]`
**my_stringlist** | `"alpha"`, `"beta"`, `"gamma"` | `device_property_read_string_array()` | `const char *[]`

### Step 2: Compiling the Overlay

The Device Tree Compiler (`dtc`) compiles the `.dts` source into a binary `.dtbo` blob.  
The `-@` flag is required for overlays, it preserves symbol information so the overlay can be correctly merged into the base tree at runtime.

```bash
dtc -@ -I dts -O dtb -o testoverlay.dtbo testoverlay.dts
```
|           |                 |
|-----------|-----------------|
|  `-@`     |   Enable symbols (required for overlays)
|`-I dts`   | Input format: DTS source
| `-O dtb`  | Output format: DTB binary
| `-o testoverlay.dtbo` | Output file

>> You may see warnings about missing `#address-cells` or `#size-cells`, these are safe to ignore for simple property-only nodes like ours.


>> Usually we add this command to `Makefile`:
```makefile
KDIR ?= /lib/modules/$(shell uname -r)/build
obj-m += dt_probe.o
OVERLAY := testoverlay

# preprocess with cpp so later overlays can #include <dt-bindings/...>
DTC_CPP := cpp -nostdinc -undef -x assembler-with-cpp -I $(KDIR)/include

all: module dtbo

module:
	$(MAKE) -C $(KDIR) M=$(CURDIR) modules

dtbo: $(OVERLAY).dtbo

%.dtbo: %.dts
	$(DTC_CPP) $< | dtc -@ -I dts -O dtb -o $@ -

clean:
	$(MAKE) -C $(KDIR) M=$(CURDIR) clean
	rm -f $(OVERLAY).dtbo
```

### Step 3: Apply the Overlay (U-Boot) and Remove

On the Raspberry Pi, `dtoverlay` patches the *running* kernel's tree. On the BeagleBone Black the overlay is
merged by **U-Boot** before Linux starts, so the flow is *install → reboot → load module*.

#### 3.1 Manually

```bash
sudo cp testoverlay.dtbo /lib/firmware/
sudo nano /boot/uEnv.txt
```
make sure these lines are present / uncommented:
```
enable_uboot_overlays=1
uboot_overlay_addr4=/lib/firmware/testoverlay.dtbo
```
`uboot_overlay_addr0..3` are used by U-Boot's cape auto-detection – use **addr4..addr7** for your own overlays.

```bash
sudo reboot
```
After reboot:
```bash
ls /proc/device-tree/my_device/          # our node is now part of the live tree
cat /proc/device-tree/my_device/label    # TestDevice
sudo beagle-version | grep -i overlay    # what U-Boot applied (BeagleBoard.org images)
```

To remove it, comment the `uboot_overlay_addr4=` line and reboot.

> **Boot broke?** A bad overlay usually makes U-Boot skip it (look for *"failed to apply"* on the serial console).
> If the board doesn't come up, put the SD card in a PC and fix `/boot/uEnv.txt` (it is on the rootfs partition),
> or boot from eMMC and mount the SD card.

#### 3.2 With the helper script (what `make install` does)

```bash
sudo ../tools/bbb_overlay.sh enable testoverlay.dtbo   # copies to /lib/firmware, picks a free addr4..7 slot
sudo ../tools/bbb_overlay.sh list                      # show overlay lines in uEnv.txt
sudo ../tools/bbb_overlay.sh disable testoverlay       # comment it out again
sudo reboot
```

#### 3.3 Runtime overlays (only if your kernel supports it)

Some custom kernels carry the out-of-tree *configfs overlay* patch. If `/sys/kernel/config/device-tree/overlays`
exists on your board you can apply without rebooting:
```bash
sudo mkdir -p /sys/kernel/config/device-tree/overlays/myoverlay
sudo cp testoverlay.dtbo /sys/kernel/config/device-tree/overlays/myoverlay/dtbo
sudo rmdir /sys/kernel/config/device-tree/overlays/myoverlay      # remove
```
(`../tools/bbb_overlay.sh runtime-load|runtime-unload` wraps this.) Stock BeagleBoard.org kernels don't have it.

>> Note: the chapter `Makefile` wraps all of this:
```makefile
install: dtbo          # copy to /lib/firmware + add to /boot/uEnv.txt, then reboot
	sudo $(TOOLS)/bbb_overlay.sh enable $(OVERLAY).dtbo $(OVERLAY_OPTS)

uninstall:
	sudo $(TOOLS)/bbb_overlay.sh disable $(notdir $(OVERLAY))

load:                  # refuses to run until the node is in the live tree
	@find /proc/device-tree -name '$(DT_NODE)' | grep -q . || { echo "run 'make install' and reboot"; exit 1; }
	sudo insmod dt_probe.ko

unload:
	sudo rmmod dt_probe
```

```bash
make                 # module + testoverlay.dtbo
make install         # once
sudo reboot
make load            # dmesg shows the properties read by dt_probe
make unload
```

# Summary

#### Overlay Structure 
```
/dts-v1/;          ← DTS version marker
/plugin/;          ← Marks this file as an overlay (not a full tree)

/ {
    compatible = "ti,am335x-bone-black";  ← Target platform (informational)

    fragment@0 {                    ← Each fragment patches one target
        target-path = "/";          ← Which node to patch (root here)
        __overlay__ {               ← Content to merge in
            my_device { ... };      ← New node added to the tree
        };
    };
};
```

Key points:

* `/plugin/;` tells dtc this is an overlay, required for the `-@` flag to work correctly
* `fragment@0` wraps each discrete patch, we can have multiple fragments targeting different nodes
* `target-path` specifies the path of the existing node to attach to, `target = <&label>` can be used instead when the node has a label
* `__overlay__` contains the actual nodes and properties that get merged in