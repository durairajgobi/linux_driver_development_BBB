# ---------------------------------------------------------------------------
# BeagleBone Black build settings
#   Native build on the board : make
#   Cross-compile from a PC   : make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- KDIR=<path-to-bb-kernel>
# See 04_beaglebone_black_setup/readme.md
# ---------------------------------------------------------------------------
KDIR ?= /lib/modules/$(shell uname -r)/build

project_name := of_parse_demo

obj-m += $(project_name).o
$(project_name)-objs := src/$(project_name).o

OVERLAY      := dts/$(project_name)
OVERLAY_OPTS := --no-universal
TOOLS        := ../tools
# node the overlay adds to the live tree (checked by `make load`)
DT_NODE      := of-parse-demo

# Overlays are run through the C preprocessor first so they can use the
# kernel's dt-bindings headers (GPIO_ACTIVE_LOW, AM33XX_PADCONF(), ...).
# The headers ship with linux-headers-$(uname -r) on the board.
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

# ---------------------------------------------------------------------------
# The BeagleBone Black applies overlays in U-Boot (no `dtoverlay` like on the
# Raspberry Pi):  install = copy to /lib/firmware + add to /boot/uEnv.txt,
# then REBOOT.  load/unload only (un)load the kernel module.
# ---------------------------------------------------------------------------
install: dtbo
	sudo $(TOOLS)/bbb_overlay.sh enable $(OVERLAY).dtbo $(OVERLAY_OPTS)

uninstall:
	sudo $(TOOLS)/bbb_overlay.sh disable $(notdir $(OVERLAY))

load:
	@find /proc/device-tree -name '$(DT_NODE)' | grep -q . || { echo "overlay not active (no '$(DT_NODE)' node): run 'make install' and reboot"; exit 1; }
	@echo "load the kernel module..."
	sudo insmod $(project_name).ko

unload:
	@echo "Unload the kernel module..."
	sudo rmmod $(project_name)
