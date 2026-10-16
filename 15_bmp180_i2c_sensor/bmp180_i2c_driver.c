#include <linux/module.h>
#include <linux/init.h>
#include <linux/i2c.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MP Coding LDD");
MODULE_DESCRIPTION("A simple I2C Device driver");

struct my_data {
    char name[32];
    int i;
};

/* Two sample device profiles */
static struct my_data a = {
    .name = "Device A",
    .i = 12, 
};

static struct my_data b = {
    .name = "Device B",
    .i = 123, 
};

static struct i2c_device_id my_ids[] = {
    {"bmp180-a", (kernel_ulong_t)&a},
    {"bmp180-b", (kernel_ulong_t)&b},
    { /* essential */ },
};
MODULE_DEVICE_TABLE(i2c, my_ids);

/*
 * Probe function
 * Called when:
 *   echo bmp180-a 0x77 > new_device
 *   
 */
static int my_probe(struct i2c_client *client){
    const struct i2c_device_id *id = i2c_client_get_device_id(client);
    pr_info("bmp180_i2c_driver name %s\n", id->name);

    struct my_data *data = (struct my_data*)id->driver_data;
    pr_info("bmp180_i2c_driver private name-%s i-%d\n", data->name, data->i);
    
    pr_info("bmp180_i2c_driver ID:0x%x\n", i2c_smbus_read_byte_data(client, 0xD0));

    return 0;
}

/*
 * Remove function
 * Called when:
 *   echo 0x77 > delete_device
 *   
 */
static void my_remove(struct i2c_client *client){
    pr_info("bmp180_i2c_driver - removing device\n");
}

#define BMP180_ADDR		0x77

/* BeagleBone Black: I2C2 on P9_19 (SCL) / P9_20 (SDA) -> /dev/i2c-2 */
static int i2c_bus = 2;
module_param(i2c_bus, int, 0444);
MODULE_PARM_DESC(i2c_bus, "I2C adapter number the BMP180 is wired to (default 2 = P9_19/P9_20)");

static struct i2c_adapter *my_adapter;
static struct i2c_client *my_client;

static struct i2c_board_info my_i2c_device_info = {
	I2C_BOARD_INFO("bmp180-a", BMP180_ADDR),
};

/* I2C driver structure */
static struct i2c_driver my_driver = {
    .probe = my_probe,
    .remove = my_remove,
    .id_table = my_ids,
    .driver = {
        .name = "bmp180-i2c-driver",
    }

};

static int __init my_init(void){
    int ret = i2c_add_driver(&my_driver);
    if(ret){
        pr_err("bmp180-i2c-driver: failed to add driver\n");
        return ret;
    }
    pr_info("bmp180-i2c-driver: created\n");

    /* Get I2C adapter (i2c-2 on the BeagleBone Black) */
	my_adapter = i2c_get_adapter(i2c_bus);
	if(!my_adapter){ /* i2c_get_adapter() returns NULL on failure */
		pr_err("bmp180_i2c_driver: failed to get adapter on i2c bus %d\n", i2c_bus);
        i2c_del_driver(&my_driver);
		return -ENODEV;
	}

    /* Create the I2C device */
	my_client = i2c_new_client_device(my_adapter, &my_i2c_device_info);
	if(IS_ERR(my_client)){
        pr_err("bmp180_i2c_driver: faild to create new I2C client\n");
		i2c_del_driver(&my_driver);
		return PTR_ERR(my_client);
	}

    i2c_put_adapter(my_adapter);

	pr_info("bmp180_i2c_driver: I2C device created automatically\n");

    return 0;
}

static void __exit my_exit(void){
    if (my_client)
		i2c_unregister_device(my_client);
    i2c_del_driver(&my_driver);
    pr_info("bmp180-i2c-driver: exit\n");
}

module_init(my_init);
module_exit(my_exit);

// module_i2c_driver(my_driver);
