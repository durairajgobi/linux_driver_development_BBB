#include <linux/module.h>
#include <linux/slab.h>    // for kmalloc, kzalloc
#include <linux/vmalloc.h> // for vmalloc
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MPCoding - LDD");
MODULE_DESCRIPTION("Memory Allocation Example");

static char *kmalloc_ptr;
static char *kzalloc_ptr;
static char *vmalloc_ptr;

#define ALLOC_SIZE_SMALL  1024     // small allocation
#define ALLOC_SIZE_LARGE  (1024 * 1024)  // 1 MB, large allocation

struct my_object {
    int id;
    char name[32];
};

static struct kmem_cache *my_cache;
static struct my_object  *obj1;

static int __init my_init(void){
    phys_addr_t phys;

    // kmalloc: physically contiguous, no zeroing
    kmalloc_ptr = kmalloc(ALLOC_SIZE_SMALL, GFP_KERNEL);
    if (!kmalloc_ptr) {
        pr_err("kmalloc failed\n");
        return -ENOMEM;
    }
    /* phys_addr_t is 32-bit on the BeagleBone Black (ARMv7, no LPAE) and 64-bit on
     * arm64 boards - %pa prints it correctly on both (pass a pointer to it). */
    phys = virt_to_phys(kmalloc_ptr);
    pr_info("kmalloc allocated at %px (physical %pa)\n",
            kmalloc_ptr, &phys);

    // kzalloc: same as kmalloc but zeros memory
    kzalloc_ptr = kzalloc(ALLOC_SIZE_SMALL, GFP_KERNEL);
    if (!kzalloc_ptr) {
        pr_err("kzalloc failed\n");
        goto label_kmalloc_ptr_free;
    }
    pr_info("kzalloc allocated at %px\n", kzalloc_ptr);

    // vmalloc: virtually contiguous, suitable for large blocks
    vmalloc_ptr = vmalloc(ALLOC_SIZE_LARGE);
    if (!vmalloc_ptr) {
        pr_err("vmalloc failed\n");
        goto label_kzalloc_ptr_free;
    }
    pr_info("vmalloc allocated at %px\n", vmalloc_ptr);

    /* slab allocation */
    my_cache = kmem_cache_create("my_cache", 
                                 sizeof(struct my_object), 
                                 0, 
                                 SLAB_HWCACHE_ALIGN | SLAB_NO_MERGE,
                                 NULL);

    if(!my_cache) goto label_vmalloc_ptr_free;

    obj1 = kmem_cache_alloc(my_cache, GFP_KERNEL);
    if(!obj1){
        goto label_kmem_cache_free;
    }

    obj1->id = 1;
    strcpy(obj1->name, "slab_object");

    pr_info("Slab object allocated at %px\n", obj1);

    return 0;

label_kmem_cache_free:
    kmem_cache_destroy(my_cache);
label_vmalloc_ptr_free:
    vfree(vmalloc_ptr);
label_kzalloc_ptr_free:
    kfree(kzalloc_ptr);
label_kmalloc_ptr_free:
    kfree(kmalloc_ptr);

    return -ENOMEM;
}

static void __exit my_exit(void){
    kfree(kmalloc_ptr);
    kfree(kzalloc_ptr);
    vfree(vmalloc_ptr);
    kmem_cache_free(my_cache, obj1);
    kmem_cache_destroy(my_cache);

    pr_info("Memory freed\n");
}

module_init(my_init);
module_exit(my_exit);

