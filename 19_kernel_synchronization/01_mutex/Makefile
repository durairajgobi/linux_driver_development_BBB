# ---------------------------------------------------------------------------
# BeagleBone Black build settings
#   Native build on the board : make
#   Cross-compile from a PC   : make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- KDIR=<path-to-bb-kernel>
# See 04_beaglebone_black_setup/readme.md
# ---------------------------------------------------------------------------
KDIR ?= /lib/modules/$(shell uname -r)/build

obj-m += my_mutex.o

all:
	$(MAKE) -C $(KDIR) M=$(CURDIR) modules

clean:
	$(MAKE) -C $(KDIR) M=$(CURDIR) clean
