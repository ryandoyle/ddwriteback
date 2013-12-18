#undef __KERNEL__
#define __KERNEL__
#undef MODULE
#define MODULE

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/kthread.h>

static struct task_struct *ddwriteback_thread;

static int ddwriteback_kthread_runner(void *params) {
  printk(KERN_INFO "ddwriteback_kthread_runner: Started\n");

  while (!kthread_should_stop()) {
    // Do work
    printk(KERN_INFO "ddwriteback_kthread_runner: in loop\n");

    // Sleep for 1 second
    set_current_state(TASK_INTERRUPTIBLE);
    schedule_timeout(HZ);
  }

  printk(KERN_INFO "ddwriteback_kthread_runner: thread exiting\n");
  return 0;
}

static int __init ddwriteback_init(void) {
  printk(KERN_INFO "ddwriteback_init: Initialising ddwriteback\n");

  ddwriteback_thread = kthread_run(ddwriteback_kthread_runner, NULL, "ddwriteback");
  if(IS_ERR(ddwriteback_thread)){
    printk(KERN_ERR "ddwriteback_init: Cannot create ddwriteback thread:%ld\n", PTR_ERR(ddwriteback_thread));
    return PTR_ERR(ddwriteback_thread);
  }

  return 0;
}

static void __exit ddwriteback_cleanup(void) {
  printk(KERN_INFO "Cleaning up module\n");
  kthread_stop(ddwriteback_thread);
}

module_init(ddwriteback_init);
module_exit(ddwriteback_cleanup);
