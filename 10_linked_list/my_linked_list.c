#include <linux/module.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/slab.h>   /* kmalloc/kfree - not pulled in implicitly on 32-bit ARM */

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MPCoding - LDD");
MODULE_DESCRIPTION("Kernel linked list Example");

static const char *device_name = "my_linked_list";

struct my_data {
    uint32_t id;
    char name[32];
    struct list_head list;
};

/* Create and initilize an empty list */
static LIST_HEAD(my_list); // my_list -> list_head at comple time

static int __init my_init(void) {

    struct my_data *tmp, *next;
    struct list_head *ptr;

    tmp = kmalloc(sizeof(struct my_data), GFP_KERNEL);
    if(!tmp){
        goto mem_free;
    }

    tmp->id = 0;
    strcpy(tmp->name, "MP Coding");
    list_add_tail(&tmp->list, &my_list);


    tmp = kmalloc(sizeof(struct my_data), GFP_KERNEL);
    if(!tmp){
        goto mem_free;
    }

    tmp->id = 1;
    strcpy(tmp->name, "Hello World");
    list_add_tail(&tmp->list, &my_list);


    tmp = kmalloc(sizeof(struct my_data), GFP_KERNEL);
    if(!tmp){
        goto mem_free;
    }

    tmp->id = 2;
    strcpy(tmp->name, "Madhawa Polkotuwa");
    list_add_tail(&tmp->list, &my_list);

    /* iterate */
    list_for_each(ptr, &my_list){
        tmp = list_entry(ptr, struct my_data, list);
        pr_info("%s: Element id:%d name:%s \n", device_name, tmp->id, tmp->name);
    }

    pr_info("%s: Added 3 elements to the list!\n", device_name);
    pr_info("%s: exit init\n", device_name);
    
    return 0;

mem_free:
    list_for_each_entry_safe(tmp, next, &my_list, list){
        list_del(&tmp->list);
        kfree(tmp);
    }

    pr_info("%s: kmalloc faild - abort init\n", device_name);
    return -ENOMEM;

}

static void __exit my_exit(void){

    struct my_data *tmp, *next;
    list_for_each_entry_safe(tmp, next, &my_list, list){
        list_del(&tmp->list);
        kfree(tmp);
    }

    pr_info("%s: Goodbye Kernel\n", device_name);
}

module_init(my_init);
module_exit(my_exit);