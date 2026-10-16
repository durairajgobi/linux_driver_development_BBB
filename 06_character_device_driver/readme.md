# Understanding Linux Device Files

## Video :
[![Youtube Video](https://img.youtube.com/vi/MZDzAPh2DhA/0.jpg)](https://www.youtube.com/watch?v=MZDzAPh2DhA) 

## Block and Character Devices

## Major & Minor 

### Where do device files live and why are they “virtual”?

* The `/dev` directory contains special files called device nodes.
```
ls /dev
ls /dev | wc -l
```


* b = block, c = character
```
ls -lh /dev/mmcblk0 /dev/tty0 /dev/gpiochip0
```

* minor number
```
ls -lh /dev/tty{1,2,3,4,5}
```

* get registerd major numbers
```
cat /proc/devices
```

### filename doesn’t matter

**Character Device: Serial Port example**
* Enable a serial port on the BeagleBone Black

  `/dev/ttyS0` (UART0, header J1) is the **Linux console** on the BBB – don't use it for this test.
  Use **UART1** on the P9 header instead: P9_24 = TXD, P9_26 = RXD → `/dev/ttyS1`.
```
config-pin P9_24 uart
config-pin P9_26 uart
ls -l /dev/ttyS1        # crw-rw---- 1 root dialout 4, 65 ...
```
> If `/dev/ttyS1` doesn't echo, enable the stock UART1 overlay instead: add
> `uboot_overlay_addr4=/lib/firmware/BB-UART1-00A0.dtbo` to `/boot/uEnv.txt` and reboot (see chapter 25).

* install screen
```
sudo apt install screen -y
```

* screen serial port (short **P9_24 and P9_26** with a jumper wire)
```
sudo screen /dev/ttyS1 9600
```
*Whatever we type anyting it will echo back, showing that the serial interface works.*   
<small>exit form screen: `Ctrl + A` , Then, `press D`.</small>

* Create `my_serial` device node (same major/minor as `ttyS1`: 4, 65)
```
sudo mknod my_serial c 4 65
```
* again screen serail bus using new node `my_serial`:
```
sudo screen my_serial 9600
```
<small>This works exact same. like `ttyS1`</small>

**The filename doesn’t matter – only major+minor do.**

**Block Device: SD Card Memeory block example**

> On the BeagleBone Black booted from SD, `mmcblk0` is the micro-SD card and `mmcblk1` the on-board eMMC (`lsblk`).
* dump the `mmcblk0` 4kb memory,
```
sudo hexdump -C /dev/mmcblk0 -n 512 | head
```

* create our own device node:
```shell
sudo mknod my_mblk b 179 0 # 179,0 is the same major and minor number of the mmcblk0
```
```
ls -lh my_mblk
```
* Read 1st 4kb of data of new device node `my_mblk`
```
sudo hexdump -C my_mblk -n 512 | head
```
