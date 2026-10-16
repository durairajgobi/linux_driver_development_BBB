#!/bin/bash
# ---------------------------------------------------------------------------
# bbb_overlay.sh - manage Device Tree overlays on a BeagleBone Black
#
# The Raspberry Pi has `dtoverlay` + /boot/firmware/config.txt.
# The BeagleBone Black (BeagleBoard.org Debian images) applies overlays in
# U-Boot, configured from /boot/uEnv.txt:
#
#     enable_uboot_overlays=1
#     uboot_overlay_addr4=/lib/firmware/my_overlay.dtbo
#     ...
#     uboot_overlay_addr7=/lib/firmware/other.dtbo
#
# Slots addr0..addr3 are normally used by U-Boot's cape auto-detection,
# addr4..addr7 are free for custom overlays, so this script uses those.
#
# Usage:
#   sudo bbb_overlay.sh enable  <file.dtbo> [--no-universal]
#   sudo bbb_overlay.sh disable <name|file.dtbo>
#   sudo bbb_overlay.sh list
#   sudo bbb_overlay.sh runtime-load   <file.dtbo>   # only if configfs overlays exist
#   sudo bbb_overlay.sh runtime-unload <name>
#
# --no-universal comments out `enable_uboot_cape_universal=1`.  The
# cape-universal overlay claims almost every header pin through
# "bone-pinmux-helper" nodes, so it MUST be disabled before an overlay
# can apply its own pinctrl settings (chapters 26, 29, 30, 32-spi).
# ---------------------------------------------------------------------------
set -e

UENV=${UENV:-/boot/uEnv.txt}
FW=/lib/firmware
CFS=/sys/kernel/config/device-tree/overlays

die() { echo "bbb_overlay: $*" >&2; exit 1; }
need_root() { [ "$(id -u)" -eq 0 ] || die "run with sudo"; }

base() { local b; b=$(basename "$1"); echo "${b%.dtbo}"; }

cmd_enable() {
    need_root
    local src="$1" opt="$2"
    [ -f "$src" ] || die "no such file: $src"
    [ -f "$UENV" ] || die "$UENV not found (is this a BeagleBoard.org image?)"

    local name dst
    name=$(base "$src")
    dst="$FW/$name.dtbo"
    cp "$src" "$dst"
    echo "copied   $src -> $dst"

    cp "$UENV" "$UENV.bak"

    # make sure overlays are enabled
    if grep -qE '^#?enable_uboot_overlays=' "$UENV"; then
        sed -i -E 's|^#?enable_uboot_overlays=.*|enable_uboot_overlays=1|' "$UENV"
    else
        echo "enable_uboot_overlays=1" >> "$UENV"
    fi

    # already present?
    if grep -qE "^uboot_overlay_addr[0-9]=$dst\$" "$UENV"; then
        echo "already enabled in $UENV"
    else
        local slot=""
        for n in 4 5 6 7; do
            if ! grep -qE "^uboot_overlay_addr$n=" "$UENV"; then slot=$n; break; fi
        done
        [ -n "$slot" ] || die "uboot_overlay_addr4..7 are all in use, disable one first"

        if grep -qE "^#uboot_overlay_addr$slot=" "$UENV"; then
            sed -i -E "0,/^#uboot_overlay_addr$slot=.*/s||uboot_overlay_addr$slot=$dst|" "$UENV"
        else
            echo "uboot_overlay_addr$slot=$dst" >> "$UENV"
        fi
        echo "enabled  uboot_overlay_addr$slot=$dst"
    fi

    if [ "$opt" = "--no-universal" ]; then
        sed -i -E 's|^enable_uboot_cape_universal=1|#enable_uboot_cape_universal=1|' "$UENV"
        echo "disabled enable_uboot_cape_universal (pins are now free for custom pinmux)"
    fi

    echo
    echo ">>> reboot to apply:  sudo reboot"
    echo ">>> after reboot check: sudo beagle-version | grep UBOOT   (or dmesg)"
}

cmd_disable() {
    need_root
    local name
    name=$(base "$1")
    cp "$UENV" "$UENV.bak"
    if grep -qE "^uboot_overlay_addr[0-9]=.*/$name\.dtbo\$" "$UENV"; then
        sed -i -E "s|^(uboot_overlay_addr[0-9]=.*/$name\.dtbo)\$|#\1|" "$UENV"
        echo "disabled $name in $UENV  ->  reboot to apply"
    else
        echo "$name is not enabled in $UENV"
    fi
}

cmd_list() {
    echo "--- $UENV ---"
    grep -E '^(enable_uboot_overlays|uboot_overlay_addr[0-9]|enable_uboot_cape_universal|disable_uboot_overlay_)' "$UENV" || true
    if [ -d "$CFS" ]; then
        echo "--- runtime (configfs) overlays ---"
        ls "$CFS"
    fi
}

cmd_runtime_load() {
    need_root
    [ -d "$CFS" ] || die "this kernel has no configfs overlay support ($CFS missing); use 'enable' + reboot"
    local name; name=$(base "$1")
    mkdir -p "$CFS/$name"
    cat "$1" > "$CFS/$name/dtbo"
    echo "status: $(cat "$CFS/$name/status")"
}

cmd_runtime_unload() {
    need_root
    local name; name=$(base "$1")
    [ -d "$CFS/$name" ] || die "$name is not loaded at runtime"
    rmdir "$CFS/$name"
}

case "$1" in
    enable)          shift; cmd_enable "$@" ;;
    disable)         shift; cmd_disable "$@" ;;
    list)            cmd_list ;;
    runtime-load)    shift; cmd_runtime_load "$@" ;;
    runtime-unload)  shift; cmd_runtime_unload "$@" ;;
    *) sed -n '2,30p' "$0"; exit 1 ;;
esac
