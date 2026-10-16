#include <linux/module.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/err.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MPCoding");
MODULE_DESCRIPTION("Simple Multiple Kernel Thread Example");

static const char *module_name = "my_kthread";

static struct task_struct *thread_1;
static struct task_struct *thread_2;

static int t1 = 1;
static int t2 = 2;

static int thread_function(void *data){

    unsigned long counter = 0;
    int thread_id = *(int *)data;
    pr_info("%s: Thread %d started \n", module_name, thread_id);

    /*
    Main loop
    */
   while(!kthread_should_stop()){

    pr_info("%s: Thread %d running - iteration %lu\n", module_name, thread_id, counter++);

    ssleep(thread_id); // thread 1 sleep 1 second, thread 2 sleep 2 seconds

   }

   pr_info("%s: Thread %d stopping\n", module_name, thread_id);

    return 0;
}

/*
 * Module Initialization
 */
static int __init my_init(void)
{
    thread_1 = kthread_run(thread_function, &t1, "kthread_1");
    if(IS_ERR(thread_1)){
        pr_err("%s: Failed to create Thread 1\n", module_name);
        return PTR_ERR(thread_1);
    }

    thread_2 = kthread_run(thread_function, &t2, "kthread_2");
    if(IS_ERR(thread_2)){
        pr_err("%s: Failed to create Thread 2\n", module_name);
        kthread_stop(thread_1);
        return PTR_ERR(thread_2);
    }

    pr_info("%s: Both thread created successfully\n", module_name);

    return 0;
}

/*
 * Module Cleanup
 */
static void __exit my_exit(void)
{
    if(thread_1)
        kthread_stop(thread_1);
    
    if(thread_2)
        kthread_stop(thread_2);

    pr_info("%s: Thread stopped. \n", module_name);
}

module_init(my_init);
module_exit(my_exit);