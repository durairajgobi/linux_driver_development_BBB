#include <linux/module.h>
#include <linux/init.h>
#include <linux/hrtimer.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MPCoding");
MODULE_DESCRIPTION("A Simple High Resolution Timers Example");

#define MAX_SAMPLE 20
static u64 timestams[MAX_SAMPLE];
static int index;

static struct hrtimer my_hrtimer;
u64 start_time;

static ktime_t period;

static enum hrtimer_restart	hrtimer_callback(struct hrtimer *timer){

    if(index < MAX_SAMPLE)
        timestams[index++] = ktime_get_ns();

    hrtimer_forward_now(timer, period);

    return HRTIMER_RESTART;
}

static int __init my_init(void){
    hrtimer_init(&my_hrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    my_hrtimer.function = &hrtimer_callback;

    start_time = ktime_get_ns();

    period = ktime_set(0, 1000000); // 1ms

    hrtimer_start(&my_hrtimer, period, HRTIMER_MODE_REL);

    pr_info("hr-timer started\n");
    return 0;
}

static void __exit my_exit(void){
    hrtimer_cancel(&my_hrtimer);

    for(int i=1; i < index; i++){
        pr_info("Delta %d: %llu ns \n", i, timestams[i] - timestams[i - 1]);
    }

    pr_info("Goodbye Kernel\n");
}

module_init(my_init);
module_exit(my_exit);