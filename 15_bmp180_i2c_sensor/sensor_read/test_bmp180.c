/*
 * read_bmp180.c
 *
 * Simple userspace program to read BMP180 sysfs attributes exposed by
 * the kernel driver (temp_input and pressure_input).
 *
 * Usage:
 *  1) Provide full sysfs device dir:
 *     ./read_bmp180 /sys/bus/i2c/devices/2-0077
 *
 *  2) Or provide bus and address:
 *     ./read_bmp180 2 0x77
 *
 * Returns 0 on success, non-zero on error.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define PATH_MAXLEN 256
#define BUF_MAX 128

/* Helper: read a single integer value from a sysfs file */
static int read_sysfs_int(const char *device_dir, const char *filename, long long *out)
{
    char path[PATH_MAXLEN];
    char buf[BUF_MAX];
    FILE *f;
    int ret = -1;

    if (snprintf(path, sizeof(path), "%s/%s", device_dir, filename) >= (int)sizeof(path)) {
        fprintf(stderr, "path too long\n");
        return -1;
    }

    f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "failed to open %s: %s\n", path, strerror(errno));
        return -1;
    }

    if (!fgets(buf, sizeof(buf), f)) {
        fprintf(stderr, "failed to read %s: %s\n", path, strerror(errno));
        goto out;
    }

    /* parse signed integer */
    errno = 0;
    char *endp = NULL;
    long long val = strtoll(buf, &endp, 10);
    if ((errno != 0) || (endp == buf)) {
        fprintf(stderr, "failed to parse integer from %s\n", path);
        goto out;
    }

    *out = val;
    ret = 0;

out:
    fclose(f);
    return ret;
}

int main(int argc, char **argv)
{
    char device_dir[PATH_MAXLEN];

    if (argc == 2) {
        /* full path provided */
        strncpy(device_dir, argv[1], sizeof(device_dir));
        device_dir[sizeof(device_dir)-1] = '\0';
    } else if (argc == 3) {
        /* bus and hex address provided */
        int bus = atoi(argv[1]);
        unsigned int addr;
        if (sscanf(argv[2], "0x%x", &addr) != 1 && sscanf(argv[2], "%x", &addr) != 1) {
            fprintf(stderr, "invalid address: %s\n", argv[2]);
            return 2;
        }
        /* device directory name is like "2-0077" (bus-addr with 4-digit hex, zero padded) */
        if (snprintf(device_dir, sizeof(device_dir), "/sys/bus/i2c/devices/%d-%04x", bus, addr) >= (int)sizeof(device_dir)) {
            fprintf(stderr, "device path too long\n");
            return 2;
        }
    } else {
        fprintf(stderr, "Usage:\n  %s /sys/bus/i2c/devices/2-0077\n  %s <bus> <hex-address>   e.g. %s 2 0x77\n", argv[0], argv[0], argv[0]);
        return 2;
    }

    long long temp10, pressure;
    if (read_sysfs_int(device_dir, "temp_input", &temp10) != 0) {
        fprintf(stderr, "error reading temp_input from %s\n", device_dir);
        return 3;
    }

    if (read_sysfs_int(device_dir, "pressure_input", &pressure) != 0) {
        fprintf(stderr, "error reading pressure_input from %s\n", device_dir);
        return 4;
    }

    /* temp10 is in tenths of °C */
    double temp_c = temp10 / 10.0;
    /* pressure is in Pa, convert to hPa for human friendly */
    double pressure_hpa = pressure / 100.0;

    printf("Device: %s\n", device_dir);
    printf("Temperature: %.1f °C (raw=%lld)\n", temp_c, temp10);
    printf("Pressure:    %lld Pa (%.2f hPa)\n", pressure, pressure_hpa);

    return 0;
}


