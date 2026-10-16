#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/sched/types.h>
#include <linux/delay.h>

#include <linux/mutex.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MPCoding - LDD");
MODULE_DESCRIPTION("Kernel Synchronization: Mutex");

/*
 * Module Name (used for logging)
 */
static const char *module_name = "my_mutex";

static struct task_struct *thread_high;
static struct task_struct *thread_low;
static char shared_buffer[64];

static struct mutex my_mutex;


typedef struct task_data {
    int value;
    char name[10];
    char text[20];
}task_data;

struct task_data high_t_pdata = {
    .value = 10,
    .name = "HIGH",
    .text = "HIGH_HIGH_HIGH"
};

struct task_data low_t_pdata = {
    .value = 5,
    .name = "LOW",
    .text = "LOW_LOW_LOW"
};

static int thread_func(void *data) {
    task_data *t_data = (task_data *)data;
    
    while (!kthread_should_stop()) {

        mutex_lock(&my_mutex);

        pr_info("%s: Running %s Priority Task, Coping Data:%s \n", module_name, t_data->name, t_data->text);
        // Writing to shared "memory"
        snprintf(shared_buffer, sizeof(shared_buffer), "Active: %s", t_data->text);
        pr_info("%s: Enter %s Shared Buffer: %s\n", module_name, t_data->name, shared_buffer);

        msleep(t_data->value * 100);

        pr_info("%s: Exit %s Shared Buffer: %s\n", module_name, t_data->name, shared_buffer);

        mutex_unlock(&my_mutex);
    }
    return 0;
}

static int __init my_init(void) {
    
    pr_info("%s: Initializing threads in SCHED_NORMAL\n", module_name);

    mutex_init(&my_mutex); // initialize mutex

    thread_high = kthread_run(thread_func, &high_t_pdata, "high_norm_kthread");
    if (IS_ERR(thread_high)) {
        pr_err("%s: Failed to create thread_high\n", module_name);
        return PTR_ERR(thread_high);
    }
    set_user_nice(thread_high, -20);  // High priority

    thread_low = kthread_run(thread_func, &low_t_pdata, "low_norm_kthread");
    if (IS_ERR(thread_low)) {
        pr_err("%s: Failed to create thread_low\n", module_name);
        kthread_stop(thread_high);
        return PTR_ERR(thread_low);
    }
    set_user_nice(thread_low, -19); // Low priority

    return 0;
}

static void __exit my_exit(void) {
    if(thread_high)
        kthread_stop(thread_high);
    
    if(thread_low)
        kthread_stop(thread_low);

    mutex_destroy(&my_mutex);


    pr_info("%s: Threads stopped\n", module_name);
}

module_init(my_init);
module_exit(my_exit);