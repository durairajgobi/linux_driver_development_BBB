# Device Tree (DT)

## Video :-
[![Youtube Video](https://img.youtube.com/vi/_kgXgLwQymo/0.jpg)](https://www.youtube.com/watch?v=_kgXgLwQymo)

> The video uses the Raspberry Pi 4 device tree as the example. This readme walks through the same concepts using
> the **BeagleBone Black** (TI AM335x) device tree, which is in mainline Linux:
> `arch/arm/boot/dts/ti/omap/am335x-boneblack.dts`.

The **Device Tree** is a data structure used by the Linux kernel to describe hardware that cannot be automatically discovered - such as peripherals on embedded platforms like the BeagleBone, Raspberry Pi, NXP, Radxa, etc.
Instead of hard-coding board-specific hardware details into the kernel source, the Device Tree externalizes this information into a human-readable text format (`.dts`) that is compiled into a binary blob (`.dtb`) and passed to the kernel at boot time.

This tutorial covers:

* Why the Device Tree exists and the problem it solves
* The structure of `.dts` and `.dtsi` files
* Core properties: `status`, `compatible`, `reg`, `gpios`, `interrupts`, `clocks`, and more
* How to read and navigate a real BeagleBone Black Device Tree

-------------------------------------------------------------------------------------
* [Device trees everywhere](https://ozlabs.org/people/dgibson/papers/dtc-paper.pdf)
* [Device tree specifications](https://www.devicetree.org/specifications)

## 1. Why Does the Device Tree Exist?
**The Problem: Board-Specific Code in the Kernel**

On x86 PCs, hardware can be discovered dynamically via buses like PCI and USB. The OS interrogates the bus and the devices identify themselves.

On embedded platforms (ARM, RISC-V, etc.), peripherals are typically memory-mapped and have no self-description mechanism.
Historically, Linux handled this with board support files -> C files in `arch/arm/mach-*/` that hard-coded every platform's hardware layout.
The TI OMAP family (the BeagleBone's ancestors) had dozens of them, e.g. `arch/arm/mach-omap2/board-omap3beagle.c` for the original BeagleBoard:
```c
/* Old way -> hard-coded resources in a board file (illustrative, AM335x UART0 values) */
static struct resource uart0_resources[] = {
    {
        .start = 0x44E09000,
        .end   = 0x44E09FFF,
        .flags = IORESOURCE_MEM,
    },
    {
        .start = 72,                 /* INTC interrupt number */
        .flags = IORESOURCE_IRQ,
    },
};
```

This led to thousands of board files being merged into the kernel - a maintenance nightmare.
The AM335x (2011) was one of the SoCs that went **DT-only** in mainline: there has never been a mainline board file for the BeagleBone.

**The Solution: Device Tree**

The Device Tree, borrowed from Open Firmware (used in Sun SPARC and PowerPC systems), provides a standardized, platform-independent way to describe hardware in a tree-structured text file:

- The kernel no longer needs board-specific C code for hardware layout
- The same kernel image can boot on multiple boards with different `.dtb` files (one ARMv7 kernel boots BeagleBone Black, Green, PocketBeagle, …)
- Hardware description is maintained outside the kernel source tree
- Firmware (bootloader) passes the `.dtb` to the kernel at boot

On the BeagleBone Black the boot chain is: **ROM code → MLO (U-Boot SPL) → U-Boot**. U-Boot reads `/boot/uEnv.txt`,
loads `zImage` and the board's `.dtb`, **applies any overlays listed in `uEnv.txt`**, and passes the DTB address to the kernel in register `r2`.

```
Boot sequence:
  ROM → MLO → U-Boot → loads zImage + am335x-boneblack*.dtb (+ .dtbo overlays) → kernel parses .dtb → drivers probe devices
```
## 2. DTS File Structure

| Extension | Description |
|-----------|-------------|
| `.dts`      | Device Tree Source - human-readable, top-level board file
| `.dtsi`     | Device Tree Source Include - reusable fragments (SoC definitions)
| `.dtb`      | Device Tree Blob - compiled binary passed to the kernel
| `.dtbo`     | Device Tree Blob Overlay - loadable overlay for boot-time / runtime modification

The layering for the BeagleBone Black (mainline, `arch/arm/boot/dts/ti/omap/`):
```
am335x-boneblack.dts               ← board-level (BeagleBone Black)
    ├── am33xx.dtsi                 ← SoC-level (AM335x: CPU, INTC, interconnects)
    │       ├── am33xx-l4.dtsi      ← every L4 peripheral: UART, I2C, SPI, GPIO, pinmux ...
    │       └── am33xx-clocks.dtsi
    ├── am335x-bone-common.dtsi     ← common to all BeagleBones (LEDs, I2C0/2, eMMC/SD, PMIC)
    ├── am335x-boneblack-common.dtsi← BBB specifics (512 MB DDR3, eMMC)
    └── am335x-boneblack-hdmi.dtsi  ← HDMI framer on the BBB
```

```bash
# Compiled .dtb files live under /boot/dtbs/<kernel version>/ on BeagleBoard.org Debian
ls /boot/dtbs/$(uname -r)/ | grep boneblack
# With U-Boot overlays enabled BeagleBoard.org images boot the "-uboot" / "-uboot-univ" variants of the BBB dtb

# Overlays (.dtbo) live in /lib/firmware
ls /lib/firmware/*.dtbo | head
```

### 2.2 Basic DTS Syntax

**Decompile a `.dtb` Back to Human-Readable `.dts`**
```bash
# Install device tree compiler - if not installed
sudo apt install device-tree-compiler

# Decompile the running board's .dtb
dtc -I fs -O dts -s /sys/firmware/devicetree/base > bbb.dts
```

A `.dts` file describes hardware as a tree of nodes and properties (simplified from the real BBB tree):

```json
/dts-v1/;                          /* DTS format version — always required */

/ {                                /* Root node */
    model = "TI AM335x BeagleBone Black";
    compatible = "ti,am335x-bone-black", "ti,am335x-bone", "ti,am33xx";

    #address-cells = <1>;          /* How many cells describe an address */
    #size-cells = <1>;             /* How many cells describe a size */

    cpus {
        #address-cells = <1>;
        #size-cells = <0>;

        cpu@0 {
            device_type = "cpu";
            compatible = "arm,cortex-a8";
            reg = <0>;             /* CPU index */
        };
    };

    memory@80000000 {
        device_type = "memory";
        reg = <0x80000000 0x20000000>;    /* Base address + size (512 MB DDR3) */
    };

    ocp {                          /* On-Chip Peripherals - the SoC bus */
        compatible = "simple-pm-bus";
        #address-cells = <1>;
        #size-cells = <1>;
        ranges;                    /* Identity mapping of addresses */

        uart0: serial@44e09000 {
            compatible = "ti,am3352-uart", "ti,omap3-uart";
            reg = <0x44e09000 0x1000>;
            interrupts = <72>;
            status = "okay";
        };
    };
};
```

> In the real `am33xx-l4.dtsi` each peripheral sits inside a `target-module@...` node (the TI *sysc* interconnect
> wrapper that handles its clocks/resets) and uses addresses relative to it, e.g. `uart0: serial@0 { reg = <0x0 0x1000>; }`.
> The flattened example above shows the same information without that extra level.

Key syntax rules:

* Every node has the form `name@unit-address { ... };`
* The `@unit-address` must match the first value of the reg property
* Properties are `key = <value>;` for cells or `key = "string";` for strings
* Labels (`uart0:`) create phandles for cross-referencing nodes

-----------

## 3. Core Properties

### 3.1 `compatible`
The most important property. It tells the kernel which driver to bind to the device. It is a list of strings ordered from most specific to most generic:

```json
compatible = "ti,am3352-uart", "ti,omap3-uart";
/*            ^-- specific SoC  ^-- generic fallback (the OMAP3 UART IP the AM335x reuses) */
```

The kernel's driver matching logic:

1. The driver declares an `of_match_table` with compatible strings it handles
2. The kernel walks the Device Tree and calls the driver's `probe()` when a match is found

```c
/* Driver side (drivers/i2c/busses/i2c-omap.c handles all AM335x I2C controllers) */
static const struct of_device_id omap_i2c_of_match[] = {
    { .compatible = "ti,omap4-i2c", .data = &omap4_pdata },
    { .compatible = "ti,omap3-i2c", .data = &omap3_pdata },
    /* ... */
    { }  /* sentinel */
};
MODULE_DEVICE_TABLE(of, omap_i2c_of_match);
```

### 3.2 `status`

Controls whether the kernel considers the node active:

```json
status = "okay";     /* Device is enabled — driver will probe it */
status = "disabled"; /* Device is disabled — driver will NOT probe */
status = "reserved"; /* Device is reserved for another purpose */
status = "fail";     /* Device has failed, do not use */
```

Default when omitted is `"okay"`. The SoC `.dtsi` disables most peripherals and the board file (or an overlay)
enables what is actually wired:

```json
/* am33xx-l4.dtsi */
i2c2: i2c@0 { compatible = "ti,omap4-i2c"; ... status = "disabled"; };

/* am335x-bone-common.dtsi - the BeagleBone routes I2C2 to P9_19/P9_20 */
&i2c2 {
    pinctrl-names = "default";
    pinctrl-0 = <&i2c2_pins>;
    status = "okay";
    clock-frequency = <100000>;
};

/* In a .dtbo overlay to enable SPI0 */
&spi0 {
    status = "okay";
};
```

### 3.3 `reg`
Describes memory-mapped register ranges or device addresses. Its format is defined by the parent node's `#address-cells` and `#size-cells`:

```json
/* Parent says: 1 cell for address, 1 cell for size */
#address-cells = <1>;
#size-cells = <1>;

uart0: serial@44e09000 {
    reg = <0x44e09000 0x1000>;
    /*     ^base addr  ^size (4 KiB) */
};

/* For I2C devices: address only, no size (size-cells = 0) */
#address-cells = <1>;
#size-cells = <0>;

cape_eeprom0: cape_eeprom0@54 {     /* on the BBB's i2c2 bus */
    reg = <0x54>;  /* I2C address 0x54 */
};
```

In driver code, you retrieve the base address with:

```c
struct resource *res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
void __iomem *base = devm_ioremap_resource(&pdev->dev, res);
```

Or the modern shorthand:

```c
void __iomem *base = devm_platform_ioremap_resource(pdev, 0);
```

### 3.4 gpios / gpios-related properties

GPIOs are referenced using phandles pointing to a GPIO controller. The AM335x has **four GPIO controllers**
(`gpio0` … `gpio3`, 32 lines each), so the phandle selects the bank and the first cell the line within it:

```json
/* GPIO controller (am33xx-l4.dtsi, bank 1 at 0x4804c000) */
gpio1: gpio@0 {
    compatible = "ti,omap4-gpio";
    reg = <0x0 0x1000>;
    gpio-controller;
    #gpio-cells = <2>;   /* <line number> <flags> */
    interrupt-controller;
    #interrupt-cells = <2>;
    interrupts = <98>;
};

/* A device using a GPIO (am335x-bone-common.dtsi - the USR0 LED) */
leds {
    compatible = "gpio-leds";

    led2 {
        label = "beaglebone:green:heartbeat";
        gpios = <&gpio1 21 GPIO_ACTIVE_HIGH>;
        /*        ^bank   ^line ^flags        = GPIO1_21 */
        linux,default-trigger = "heartbeat";
        default-state = "off";
    };
};
```

`GPIO_ACTIVE_HIGH` (0) and `GPIO_ACTIVE_LOW` (1) are defined in `<dt-bindings/gpio/gpio.h>`.
In a driver, reading a GPIO from the Device Tree:

```c
#include <linux/of_gpio.h>

int gpio_num = of_get_named_gpio(pdev->dev.of_node, "gpios", 0);
if (gpio_num < 0) {
    dev_err(&pdev->dev, "Failed to get GPIO\n");
    return gpio_num;
}

/* Or using the newer descriptor API */
#include <linux/gpio/consumer.h>

struct gpio_desc *gpiod = devm_gpiod_get(&pdev->dev, "led", GPIOD_OUT_LOW);
```

### 3.5 interrupts and interrupt-parent
Interrupt lines are described similarly to GPIOs. The AM335x interrupt controller (INTC) needs only **one cell**
(the interrupt number, 0–127, from the AM335x TRM) – no type flags, all INTC inputs are level-high:
```json
/* Interrupt controller (am33xx.dtsi) */
intc: interrupt-controller@48200000 {
    compatible = "ti,am33xx-intc";
    interrupt-controller;
    #interrupt-cells = <1>;  /* <IRQ number> */
    reg = <0x48200000 0x1000>;
};

uart0: serial@44e09000 {
    compatible = "ti,am3352-uart", "ti,omap3-uart";
    reg = <0x44e09000 0x1000>;
    interrupt-parent = <&intc>;   /* usually inherited from the root */
    interrupts = <72>;            /* UART0INT */
};

/* A device whose interrupt comes from a GPIO uses the GPIO bank as interrupt parent (2 cells): */
my_sensor@68 {
    interrupt-parent = <&gpio3>;
    interrupts = <19 IRQ_TYPE_EDGE_FALLING>;   /* P9_27 = GPIO3_19 */
};
```

In driver code:

```c
int irq = platform_get_irq(pdev, 0);
if (irq < 0)
    return irq;

ret = devm_request_irq(&pdev->dev, irq, my_irq_handler,
                       IRQF_SHARED, "my-device", dev);
```

### 3.6 `clocks`
Clock sources are described using phandles to a clock provider. On the AM335x every module's functional clock is
gated by a *CLKCTRL* register in the PRCM, exposed as a clock provider:

```json
/* am33xx-l4.dtsi (simplified) */
target-module@9000 {                       /* UART0 module wrapper */
    compatible = "ti,sysc-omap2", "ti,sysc";
    clocks = <&l4_wkup_clkctrl AM3_L4_WKUP_UART1_CLKCTRL 0>;   /* PRCM numbers UARTs from 1: UART0 = UART1_CLKCTRL */
    clock-names = "fck";
    ...
};

&i2c2 {
    clock-frequency = <100000>;   /* I2C bus speed: 100 kHz */
};
```

In driver code:

```c
#include <linux/clk.h>

struct clk *clk = devm_clk_get(&pdev->dev, NULL);
if (IS_ERR(clk))
    return PTR_ERR(clk);

ret = clk_prepare_enable(clk);
```

> For TI peripherals the `ti-sysc` interconnect driver enables these clocks via runtime PM, so drivers such as
> `i2c-omap` just call `pm_runtime_get_sync()` instead of handling clocks directly.

### 3.7 `#address-cells` and `#size-cells`
These meta-properties define the encoding of child reg properties. They are always specified on the parent node:

```json
ocp {
    #address-cells = <1>;   /* 1 × 32-bit word = 32-bit address */
    #size-cells = <1>;      /* 1 × 32-bit word = 32-bit size */
    ranges;
    ...
};

/* On 64-bit systems (e.g. RPi 4, BeaglePlay), 2 cells = 64-bit address */
memory-controller@80000000 {
    #address-cells = <2>;
    #size-cells = <2>;
    ...
};
```

### 3.8 `ranges`
The ranges property creates an address translation mapping between child address space and parent address space.
The AM335x tree uses it heavily so that each peripheral can be described with small, relative addresses:

```json
l4_wkup: interconnect@44c00000 {     /* L4 wakeup interconnect at CPU address 0x44c00000 */
    ...
    segment@200000 {                 /* 0x44e00000 */
        compatible = "simple-pm-bus";
        #address-cells = <1>;
        #size-cells = <1>;
        ranges = <0x00000000 0x00200000 0x002000>, ...;
        /*        ^child addr ^parent addr ^length   */
        /* child 0x0 → parent 0x200000 → CPU 0x44c00000 + 0x200000 = 0x44e00000 */
    };
};
```

An empty `ranges`; means identity mapping (child address = parent address).

### 3.9 pinctrl / Pin Multiplexing
On the BeagleBone Black each header pad can carry up to 8 functions (UART, SPI, I2C, GPIO …). The pads are
configured by the **control module** through the generic `pinctrl-single` driver (`&am33xx_pinmux`), a separate
block from the GPIO controllers (on the RPi they are the same block). Real example from `am335x-bone-common.dtsi`:
```json
&am33xx_pinmux {
    i2c2_pins: i2c2-pins {
        pinctrl-single,pins = <
            AM33XX_PADCONF(AM335X_PIN_UART1_CTSN, PIN_INPUT_PULLUP, MUX_MODE3)  /* P9_20: uart1_ctsn.i2c2_sda */
            AM33XX_PADCONF(AM335X_PIN_UART1_RTSN, PIN_INPUT_PULLUP, MUX_MODE3)  /* P9_19: uart1_rtsn.i2c2_scl */
        >;
    };
};

&i2c2 {
    pinctrl-names = "default";
    pinctrl-0 = <&i2c2_pins>;
    status = "okay";
};
```
Details in [chapter 29](../29_pinmux/).

### 3.10 Other Common Properties

Property | Description | Example |
|--------|-------------|---------|
| `model` | Human-readable board description | `"TI AM335x BeagleBone Black"` |
| `device_type` | Node class (`cpu`, `memory`) | `device_type = "memory";` |
| `label` | Human-readable name for LEDs, GPIOs | `label = "beaglebone:green:usr0";` |
| `linux,default-trigger` | LED trigger | `"heartbeat", "mmc0"` |
| `clock-frequency` | Bus/peripheral clock in Hz | `<100000>` (100 kHz) |
| `bus-width` | Bus data width in bits | `<8>` (8-bit eMMC on the BBB) |
| `dr_mode` | USB controller mode | `"host"`, `"peripheral"`, `"otg"` |
| `power-domains` | Power management domain | `<&prm_per>` |
| `gpio-line-names` | Names for each line of a GPIO bank | `"P9_12"`, `"[usr0 led]"` |


## 4. Node Naming Conventions

```json
node-name@unit-address
```
* node-name - describes the device type (e.g., `serial`, `i2c`, `gpio`, `leds`)
* unit-address - hexadecimal base address from `reg`, without `0x` prefix
* If a node has no `reg` (no address), the `@unit-address` part is omitted

Standard generic names defined in the Device Tree Specification:

```
adc, cache-controller, clock, cpu, ethernet, gpio, i2c, interrupt-controller,
memory, mmc, pci, serial, sound, spi, timer, usb, watchdog
```

## 5. Phandles and References
Nodes can reference each other using labels that resolve to phandles (integer handles):

```json
/* Defining a label */
gpio1: gpio@0 { ... };

/* Referencing it with & */
device {
    gpios = <&gpio1 28 0>;   /* &gpio1 resolves to gpio1's phandle  -> P9_12 */
};
```

The `&label` syntax is also used to extend or override existing nodes (common in overlays and board files that include SoC `.dtsi` files):

```json
/* In am33xx-l4.dtsi */
uart0: serial@0 {
    status = "disabled";   /* Off by default */
};

/* In am335x-bone-common.dtsi */
&uart0 {
    pinctrl-names = "default";
    pinctrl-0 = <&uart0_pins>;
    status = "okay";       /* Board enables it (debug console on J1) */
};
```

## 6. Shell Commands for Exploring the Device Tree on the BeagleBone Black

### 6.1 Locate Device Tree Files on the BBB
```bash
# Base .dtb files (BeagleBoard.org Debian)
ls /boot/dtbs/$(uname -r)/ | grep -i bone

# Which dtb / overlays U-Boot used
cat /boot/uEnv.txt
sudo beagle-version            # prints UBOOT: Loaded Overlay:[...] lines

# Overlays
ls /lib/firmware/*.dtbo
```

### 6.2 Decompile a `.dtb` Back to Human-Readable `.dts`

```bash
# Install device tree compiler
sudo apt install device-tree-compiler

# Decompile the running board's .dtb (includes the applied overlays)
dtc -I fs -O dts -s /sys/firmware/devicetree/base > bbb.dts

# View the result
less bbb.dts
```

### 6.3 Inspect the Live Device Tree via `/proc` and `/sys`
The kernel exposes the active Device Tree at `/proc/device-tree` (a sysfs-like interface):

```bash
# List all top-level DT nodes
ls /proc/device-tree/

# Read a string property
cat /proc/device-tree/model
# Output: TI AM335x BeagleBone Black

# Read the compatible property (null-separated strings)
cat /proc/device-tree/compatible | tr '\0' '\n'
# Output:
# ti,am335x-bone-black
# ti,am335x-bone
# ti,am33xx

# Navigate to the on-chip peripherals
ls /proc/device-tree/ocp/

# Find a node and read a binary/integer property using xxd
find /proc/device-tree -name "gpio@0" | head
xxd "$(find /proc/device-tree -path '*target-module@4c000/gpio@0' | head -1)/reg"
```

### 6.4 Use dtc to Inspect the Live Flattened DT
```bash
# Read the live FDT passed by the bootloader
dtc -I fs /proc/device-tree 2>/dev/null | less

# Search for a specific compatible string
dtc -I fs /proc/device-tree 2>/dev/null | grep -B 3 -A 6 "ti,omap4-i2c"
```

### 6.5 Find Which DT Node Corresponds to a Device
```bash
# Each platform device exposed by DT appears under /sys/bus/platform/devices/
ls /sys/bus/platform/devices/ | grep -E "gpio|i2c|serial"
# Output: 44e07000.gpio  4804c000.gpio  481ac000.gpio  481ae000.gpio  4819c000.i2c  44e09000.serial ...

# The of_node symlink points to the DT node in sysfs
ls -la /sys/bus/platform/devices/4804c000.gpio/of_node
# → ../../../../../../firmware/devicetree/base/ocp/interconnect@48000000/segment@0/target-module@4c000/gpio@0

# Read compatible directly from sysfs
cat /sys/bus/platform/devices/4804c000.gpio/of_node/compatible | tr '\0' '\n'
```

### 6.6 Check Which Driver is Bound to a DT Node
```bash
# View bound driver
ls -la /sys/bus/platform/devices/4804c000.gpio/driver
# → ../../../../bus/platform/drivers/omap_gpio

# List all platform devices and their drivers
for dev in /sys/bus/platform/devices/*; do
    drv=$(readlink "$dev/driver" 2>/dev/null | xargs basename 2>/dev/null)
    echo "$(basename $dev) → ${drv:-no driver}"
done | sort
```

### 6.7 Load and Remove a Device Tree Overlay
On the BeagleBone Black overlays are applied by **U-Boot at boot**, configured in `/boot/uEnv.txt`
(details in [chapter 25](../25_device_tree_overlay/)):
```bash
# List available overlays
ls /lib/firmware/ | grep dtbo

# Enable one (e.g. the stock SPI0 spidev overlay) in a free custom slot
sudo nano /boot/uEnv.txt
#   enable_uboot_overlays=1
#   uboot_overlay_addr4=/lib/firmware/BB-SPIDEV0-00A0.dtbo
sudo reboot

# Or with the helper script in this repo
sudo ./tools/bbb_overlay.sh enable /lib/firmware/BB-SPIDEV0-00A0.dtbo
sudo ./tools/bbb_overlay.sh list
```

Kernels that carry the (out-of-tree) configfs overlay patch also allow runtime loading through
`/sys/kernel/config/device-tree/overlays/`; stock BeagleBoard.org kernels don't, so the
`dtoverlay`/configfs flow from the Raspberry Pi video is replaced by *edit `uEnv.txt` + reboot*.

### 6.8 Compile Your Own .dts File
```bash
# Write a minimal overlay: P9_12 (GPIO1_28) pad -> GPIO output
cat > ~/my_overlay.dts << 'EOF'
/dts-v1/;
/plugin/;

#include <dt-bindings/pinctrl/am33xx.h>

/ {
    compatible = "ti,am335x-bone-black", "ti,am335x-bone", "ti,am33xx";

    fragment@0 {
        target = <&am33xx_pinmux>;
        __overlay__ {
            my_pins: my-pins {
                pinctrl-single,pins = <
                    AM33XX_PADCONF(AM335X_PIN_GPMC_BEN1, PIN_OUTPUT, MUX_MODE7)  /* P9_12 */
                >;
            };
        };
    };
};
EOF

# Preprocess (for the #include) and compile to .dtbo
cpp -nostdinc -undef -x assembler-with-cpp -I /lib/modules/$(uname -r)/build/include ~/my_overlay.dts \
    | dtc -@ -I dts -O dtb -o ~/my_overlay.dtbo -

# Inspect the compiled blob
dtc -I dtb -O dts ~/my_overlay.dtbo
```

### 6.9 Find DT Bindings Documentation
```bash
# Kernel DT bindings are in Documentation/devicetree/bindings/
# The board usually has no kernel source, browse online:
# https://www.kernel.org/doc/Documentation/devicetree/bindings/
#   gpio/gpio-omap.txt, i2c/ti,omap4-i2c.yaml, spi/omap-spi.yaml, pinctrl/pinctrl-single.yaml

# The BBB device tree sources themselves:
# https://github.com/torvalds/linux/tree/master/arch/arm/boot/dts/ti/omap  (am335x-boneblack*.dts, am33xx*.dtsi)

# The dt-bindings headers used by .dts files ship with the kernel headers:
ls /lib/modules/$(uname -r)/build/include/dt-bindings/pinctrl/am33xx.h
```

## 7. How the Kernel Uses the Device Tree

```
Boot
 │
 ├─ U-Boot loads .dtb (+ applies .dtbo overlays) into memory, passes address in r2 (ARM)
 │
 ├─ setup_arch() → unflatten_device_tree()
 │       Parses DTB into kernel's internal tree structure (struct device_node)
 │
 ├─ of_platform_populate()
 │       Walks the DT, creates platform_device for each node with a compatible
 │       string that matches a registered of_device_id
 │
 └─ Driver's .probe() is called with platform_device
         Driver uses of_property_read_*() / devm_gpiod_get() / etc.
         to read properties from the DT node
```

In a driver, the `struct device_node` is accessed via:
```c
static int my_probe(struct platform_device *pdev)
{
    struct device_node *np = pdev->dev.of_node;
    u32 frequency;
    const char *name;

    /* Read a u32 property */
    if (of_property_read_u32(np, "clock-frequency", &frequency))
        frequency = 100000;  /* default */

    /* Read a string property */
    if (of_property_read_string(np, "label", &name))
        name = "unknown";

    /* Check for a boolean property (presence = true) */
    if (of_property_read_bool(np, "big-endian"))
        flags |= MY_FLAG_BIG_ENDIAN;

    return 0;
}
```

## 8. Summary

Concept | Key Points
--------|-----------|
Why DT exists | Eliminates hard-coded board files; same kernel, different `.dtb`
`.dts` | Human-readable source; top-level board description (`am335x-boneblack.dts`)
`.dtsi` | Reusable SoC/platform fragments, included by `.dts` (`am33xx.dtsi`, `am335x-bone-common.dtsi`)
`.dtb` | Compiled binary passed to kernel by U-Boot
`.dtbo` | Overlay binary, applied by U-Boot from `/boot/uEnv.txt` on the BBB
`compatible` | Drives driver matching; most specific → most generic
`status` | `"okay"` enables, `"disabled"` suppresses driver probe
`reg` |  Memory-mapped base address + size (format set by `#address-cells`/`#size-cells`)
`gpios` | Phandle of the GPIO **bank** (`&gpio0..3`) + line number + flags
`interrupts` | IRQ number (1 cell on the AM335x INTC) or GPIO-bank line + type
`clocks` | Phandle reference to clock provider + clock ID (PRCM CLKCTRL on the AM335x)
`ranges` | Address translation from child to parent bus address space
`pinctrl-*` | Pad mux/config via `&am33xx_pinmux` (pinctrl-single)
Phandles | Labels (`&name`) allow cross-node references and node extension
