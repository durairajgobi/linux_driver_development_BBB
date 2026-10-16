#include <linux/module.h>
#include <linux/spi/spi.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MPCoding LDD");
MODULE_DESCRIPTION("Simple SPI Master");

#define SPI_MAX_HZ  1000000
// spi0.0

static int my_probe(struct spi_device *spi){
    pr_info("probe called...\n");

    spi->mode = SPI_MODE_0;
    spi->max_speed_hz = SPI_MAX_HZ;
    spi_setup(spi);

    u8 tx = 0x12;
    u8 rx = 0x00;

    struct spi_transfer t = {
        .tx_buf = &tx,
        .rx_buf = &rx,
        .len = 1, // 8bit
    };

    struct spi_message m;

    spi_message_init(&m);
    spi_message_add_tail(&t, &m);

    int ret = spi_sync(spi, &m);
    if(ret){
        pr_err("SPI trnsfer failed\n");
        return ret;
    }

    pr_info("SPI Trnsfer completed Rx value : 0x%02x \n", rx);

    return 0;
}

static void	my_remove(struct spi_device *spi){
    pr_info("remove called...\n");
}

static const struct spi_device_id my_ids[] = {
    { "mydevice", 0 },
    { }
};
MODULE_DEVICE_TABLE(spi, my_ids);

static struct spi_driver my_spi_driver = {
    .driver = {
        .name = "my_spi_driver",
    },
    .probe = my_probe,
    .remove = my_remove,
    .id_table = my_ids,
};

// static int __init my_init(void){
//     return 0;
// }

// static void __exit my_exit(void){

// }

module_spi_driver(my_spi_driver);