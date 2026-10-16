# Linux I2C Subsystem 

### Video :
[![Youtube Video](https://img.youtube.com/vi/r5IYB4xjk2o/0.jpg)](https://www.youtube.com/watch?v=r5IYB4xjk2o)

# 1️⃣ Introduction

I²C is a synchronous serial communication protocol that uses a simple two-wire interface to connect low-speed peripheral devices. It is commonly used to communicate with devices such as sensors, EEPROMs, real-time clocks, ADCs, DACs, and power-management ICs.

The protocol was originally developed by Philips, and today it is supported by almost all major IC manufacturers.

*In Linux-based embedded systems, I²C plays a very important role because many external peripherals are connected using this bus.*

**Widely used devices:**    
* Sensors
* EEPROMs
* RTCs
* Power-management ICs

**Linux I2C:**   
Linux provides a generic I²C framework that abstracts the hardware details.

This means that as driver developers, we do not directly control the I²C hardware. Instead, we interact with the Linux I²C subsystem, which handles communication in a standardized way.

# 2️⃣ I2C Bus Basics

![](i2c_bus.jpg)    

The I²C bus is a two-wire bus consisting of SDA and SCL lines.  
SDA is the data line, and SCL is the clock line. Both lines are open-drain and require pull-up resistors.   
I²C supports multi-master and multi-slave configurations. Each device connected to the bus has a unique address, which can be either 7-bit or 10-bit.


During an I²C transaction, the master device generates the clock on the SCL line and initiates either a read or write operation. After every 8 bits of data, the receiver sends an ACK or NACK bit to indicate whether the transfer was successful.

![](i2c_start_stop.png)
![](i2c_data_transfer.png)

* Single bus shared by multiple devices
* Masters and slaves connected in parallel
* Pull-up resistors on SDA and SCL
* Only one master controls the bus at a time

# 3️⃣ Linux I2C Subsystem

Now let’s look at how Linux models this I²C bus internally.

The Linux I²C subsystem is the interface through which the kernel communicates with devices connected to the I²C bus. In Linux, the kernel always acts as the I²C master.


![](I2C_Subsystem_1.png)

The I²C subsystem is logically divided into two main parts: `buses` and `devices`.  

* On the bus side, we have `adapters` and `algorithms`.   
* On the device side, we have `clients` and `drivers`.

* **Bus side**
    * `Adapter` → represents the I²C controller hardware
    * `Algorithm` → defines how transfers are performed
* **Device side**
    * `Client` → represents a physical I²C slave device
    * `Driver` → contains device-specific logic

# 4️⃣ Linux I2C Subsystem Architecture (Kernel View)
![](I2C_Subsystem_2.png)

At the center of the subsystem is the I²C core.

The I²C core is responsible for registering adapters, clients, and drivers, and for matching I²C devices with the appropriate drivers.

# 5️⃣ The Three Main Components

## 1️⃣ I2C Core (i2c-core)
The I²C core provides common infrastructure for all I²C drivers.
It handles:
* Registration and removal of I²C adapters.
* Registration and removal of I²C client drivers.
* Device–driver matching.
* Common I²C APIs such as i2c_transfer().

## 2️⃣ I2C Bus Driver (Adapter / Algorithm)
The I²C bus driver represents the I²C controller hardware present on the SoC.

It is responsible for generating clock signals, handling start and stop conditions, and physically transferring data over the SDA and SCL lines.

*This part of the subsystem is described using two main structures:*
* struct i2c_adapter
* struct i2c_algorithm  

*Typically, the adapter driver is implemented as a platform driver.*

## 3️⃣ I2C Client Driver
The I²C client driver represents a specific slave device connected to the I²C bus, such as a sensor or EEPROM.

*An I²C client driver consists of:*
* struct i2c_driver → driver logic
* struct i2c_client → device instance   

*When the I²C core finds a match between a client and a driver, it calls the driver’s probe() function.*

# 6️⃣ Location of I2C Code in the Kernel
All I²C-related code in the Linux kernel is located under the `drivers/i2c` directory.

`i2c-core.h`
→ Core I²C framework

`i2c-dev.c`
→ Generic user-space I²C access via /dev/i2c-*

`drivers/i2c/busses/`
→ I²C adapter drivers

`drivers/i2c/chips/`
→ Legacy or specific device drivers

`drivers/i2c/algos/`
→ I²C transfer algorithms

# 7️⃣ Two Ways to Use I2C in Linux
## Method 1️⃣ Using `i2c-dev` (User Space Control)

The first method is to use the generic `i2c-dev` driver.

This exposes I²C adapters as device files like `/dev/i2c-2`, allowing user-space applications to directly communicate with I²C devices.
* Used for testing and prototyping
* Common tools: i2c-tools

## Method 2️⃣ Writing a Dedicated I2C Client Driver
The second method is to write a dedicated kernel-space I²C client driver for a specific device.

* Recommended for production systems
* Better performance and integration
* Fully managed by the kernel

*In this video series, we will focus on writing proper I²C client drivers.*


-----------------------------------------------------------------
-----------------------------------------------------------------
-----------------------------------------------------------------

**Now that we understand the Linux I²C subsystem conceptually, let’s look at how I²C appears in a running Linux system.**

> ### I²C on the BeagleBone Black
> The AM335x has three I²C controllers (`omap-i2c` driver). On the stock BeagleBone Black device tree:
>
> | Bus | Pins | Default state | Already on the bus |
> |---|---|---|---|
> | `i2c-0` | internal | enabled, 400 kHz | TPS65217 PMIC (0x24), board EEPROM (0x50), HDMI framer (0x70) - **don't experiment here** |
> | `i2c-1` | P9_17/P9_18 or P9_24/P9_26 | disabled (needs an overlay) | - |
> | `i2c-2` | **P9_19 (SCL) / P9_20 (SDA)** | enabled, 100 kHz | cape EEPROMs 0x54–0x57 |
>
> All examples in this series use **`i2c-2`**. The BBB has on-board pull-ups on I2C2, so a BMP180 breakout
> can be wired directly (VCC → P9_3, GND → P9_1). `i2cdetect -y -r 2` typically shows `UU` at 0x54–0x57 (cape EEPROM nodes bound by the at24 driver).

## 1️⃣ Listing I²C Devices in User Space (/dev)
```bash
ls /dev/
```
Each `/dev/i2c-*` file represents an I²C `adapter` exposed to `user space`.

*For example, `i2c-2` means I²C adapter number 2. These device nodes are created by the `i2c-dev` driver.*   
*Applications and tools like `i2c-tools` use these device files to communicate with I²C devices directly from user space.*   
note: (to install i2c-tool -> `sudo apt-get install i2c-tools`)

## 2️⃣ Listing Available I²C Adapters
```bash
i2cdetect -l
```
* This command shows all I²C adapters registered with the kernel.
* It tells us which I²C controller driver created each adapter.

## 3️⃣ Scanning Devices on an I²C Bus
```bash
i2cdetect -y -r 2
```
* This scans I²C bus number 1 and shows all slave devices that respond to I²C addresses.
* Each detected address corresponds to a physical device connected to the bus.

## 4️⃣ Viewing I²C Configuration from Device Tree
```bash
dtc -I fs -O dts -s /sys/firmware/devicetree/base > tmp_dt.dts
cat tmp_dt.dts
```
* Most modern embedded Linux systems use Device Tree to describe hardware.
* This command converts the live device tree from the kernel into a readable DTS format.

Inside this file, we can see:
* I²C controllers
* Bus numbers
* Clock and pin configurations
* I²C devices defined at boot time

## 5️⃣ Exploring I²C via Sysfs (Kernel View)
**List I²C class devices**
```bash
ls /sys/class/i2c-dev/
```
* This directory shows all I²C adapters registered with the kernel.
* Each entry here maps directly to a /dev/i2c-* device file.

## 6️⃣ Inspecting a Specific I²C Adapter in Sysfs
```bash
ls /sys/class/i2c-dev/i2c-2/
```
* This directory links to the actual I²C adapter device in the kernel device model.

```bash
ls /sys/class/i2c-dev/i2c-2/device/
```
* Here we can see adapter-specific files, including the ability to dynamically create or remove I²C client devices.

## 7️⃣ Creating a New I²C Device Dynamically
```bash
echo eeprom 0x50 > /sys/class/i2c-dev/i2c-2/device/new_device
```

* This command dynamically creates an I²C client device on bus i2c-2 with address 0x50.
* The string eeprom must match an entry in the i2c_device_id table of a registered I²C client driver.

Once this command is executed:  
1. The kernel creates a new `i2c_client`
2. The I²C core searches for a matching driver
3. If a match is found, the driver’s `probe()` function is called

**Verify device creation**: 
```bash
ls /sys/bus/i2c/devices/
```
-> 1-0050 means bus 1, address 0x50.

## 8️⃣ Removing the Device (Cleanup)
```bash
sudo su
echo 0x50 > /sys/class/i2c-dev/i2c-2/device/delete_device
```

*This removes the dynamically created I²C client and calls the driver’s `remove()` function.*

## 9️⃣ How User Space and Kernel Space Connect

* User space interacts with I²C through `/dev/i2c-*` using the `i2c-dev` driver.
* Kernel drivers interact with I²C through the I²C core using APIs such as `i2c_add_driver()` and `i2c_transfer()`.

*Both views represent the same underlying I²C subsystem.*