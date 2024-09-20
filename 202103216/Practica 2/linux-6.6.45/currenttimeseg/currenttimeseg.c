#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/time.h>

SYSCALL_DEFINE0(currenttimeseg)
{
    struct timespec64 ts;
    ktime_get_real_ts64(&ts);
    return ts.tv_sec;
}