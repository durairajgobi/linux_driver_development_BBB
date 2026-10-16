#!/bin/sh
# ---------------------------------------------------------------------------
# bbb_gpio_num.sh - translate a BeagleBone Black header pin name (P8_xx /
# P9_xx) into the global (legacy) GPIO number used by gpio_to_desc().
#
#   $ sudo ./tools/bbb_gpio_num.sh P9_12
#   540
#
# Why this exists
# ---------------
# On kernel 4.x the AM335x GPIO banks were numbered 0..127
# (gpioB_N  ->  B*32 + N, e.g. P9_12 = gpio1_28 = 60).
# Since kernel 6.x the gpiolib core assigns bases dynamically starting at
# 512, and the order in which the four OMAP GPIO banks register has changed
# more than once between releases (6.1 / 6.6 / 6.12 / 6.15 ...).
# The BeagleBoard.org device trees name every line after its header pin
# (gpio-line-names), so the most reliable way to find the number is to look
# the name up at run time instead of hard-coding arithmetic.
# ---------------------------------------------------------------------------
set -e

PIN="$1"
if [ -z "$PIN" ]; then
    echo "usage: $0 <P8_xx|P9_xx>" >&2
    exit 1
fi

DBG=/sys/kernel/debug/gpio
if [ ! -r "$DBG" ]; then
    mount | grep -q debugfs || mount -t debugfs none /sys/kernel/debug 2>/dev/null || true
fi

# 1) debugfs:  " gpio-540 (P9_12               |...) in  lo"
if [ -r "$DBG" ]; then
    # exact name first; some trees call shared pins "P9_15A"/"P9_15B" -> fall back to the "A" line
    for n in "$PIN" "${PIN}A"; do
        NUM=$(grep -E "gpio-[0-9]+ \($n[ |\[]" "$DBG" | head -n1 | sed -E 's/^ *gpio-([0-9]+).*/\1/')
        [ -n "$NUM" ] && break
    done
    if [ -n "$NUM" ]; then
        echo "$NUM"
        exit 0
    fi
fi

# 2) libgpiod v1 (gpiofind) -> chip + offset, then chip base from sysfs
if command -v gpiofind >/dev/null 2>&1; then
    set -- $(gpiofind "$PIN" 2>/dev/null || gpiofind "${PIN}A" 2>/dev/null || true)
    CHIP="$1"; OFF="$2"
    if [ -n "$CHIP" ] && [ -n "$OFF" ]; then
        PARENT=$(readlink -f "/sys/bus/gpio/devices/$CHIP/..")
        for c in /sys/class/gpio/gpiochip*; do
            if [ "$(readlink -f "$c/device")" = "$PARENT" ]; then
                echo $(( $(cat "$c/base") + OFF ))
                exit 0
            fi
        done
    fi
fi

echo "$0: could not find '$PIN' (run as root, check 'sudo cat $DBG')" >&2
exit 1
