#undef __KERNEL__
#define __KERNEL__
#undef MODULE
#define MODULE

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/mm.h>
#include <linux/mmzone.h>
#include <linux/vmstat.h>

static struct task_struct *ddwriteback_thread;

static unsigned long kilobytes_written_from_page_cache(void) {
  return global_page_state(NR_WRITTEN) * PAGE_SIZE/1024;
}

static int ddwriteback_kthread_runner(void *params) {
  unsigned long bytes_written_previous;
  unsigned long bytes_written_now;
  unsigned long bytes_delta;
  unsigned long highest_rate;
  highest_rate = 0;

  bytes_written_previous = kilobytes_written_from_page_cache();
  printk(KERN_INFO "ddwriteback_kthread_runner: Started\n");

  while (!kthread_should_stop()) {
    // Calculate the rate
    bytes_written_now = kilobytes_written_from_page_cache();
    bytes_delta = bytes_written_now - bytes_written_previous;
    bytes_written_previous = bytes_written_now;
    
    // Is it highest rate we've seen?
    if(bytes_delta > highest_rate) {
      highest_rate = bytes_delta;
      printk(KERN_INFO "ddwriteback_kthread_runner: NEW write rate: %luKB/s\n", bytes_delta);
    }

    printk(KERN_INFO "ddwriteback_kthread_runner: write rate: %luKB/s\n", bytes_delta);

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
