#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/timekeeping.h>

SYSCALL_DEFINE0(systemuptimeseg)
{
    struct timespec64 uptime;
    ktime_get_boottime_ts64(&uptime);
    return uptime.tv_sec;
}
