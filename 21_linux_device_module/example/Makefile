# ---------------------------------------------------------------------------
# BeagleBone Black build settings
#   Native build on the board : make
#   Cross-compile from a PC   : make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- KDIR=<path-to-bb-kernel>
# See 04_beaglebone_black_setup/readme.md
# ---------------------------------------------------------------------------
KDIR ?= /lib/modules/$(shell uname -r)/build

obj-m += mybus.o
obj-m += mydevice.o
obj-m += mydriver.o

all:
	$(MAKE) -C $(KDIR) M=$(CURDIR) modules

clean:
	$(MAKE) -C $(KDIR) M=$(CURDIR) clean


load: 
	@echo "--- Loading mybus ---"
	sudo insmod mybus.ko
	@echo "--- Loading mydevice ---"
	sudo insmod mydevice.ko
	@echo "--- Loading mydriver ---"
	sudo insmod mydriver.ko

unload: 
	@echo "--- Unloading mydriver ---"
	sudo rmmod mydriver.ko
	@echo "--- Unloading mydevice ---"
	sudo rmmod mydevice.ko
	@echo "--- Unloading mybus ---"
	sudo rmmod mybus.ko
