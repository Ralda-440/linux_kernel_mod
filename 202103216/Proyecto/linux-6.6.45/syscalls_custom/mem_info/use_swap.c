#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/swap.h>
#include <linux/zswap.h>
#include <linux/vmstat.h>
#include <linux/swapfile.h>
#include <linux/uaccess.h>

SYSCALL_DEFINE1(use_swap, unsigned long __user *, _info){
    struct sysinfo mem_info;
    si_meminfo(&mem_info);
    si_swapinfo(&mem_info);
    //Informacion de memoria
    unsigned long *info = kmalloc_array(3, sizeof(unsigned long), GFP_KERNEL);
    if (!info)
    {
        return -ENOMEM;
    }
    //Swap usado en paginas
    info[0] = mem_info.totalswap - mem_info.freeswap;
    //Swap libre en paginas
    info[1] = mem_info.freeswap;
    //Swap total en paginas
    info[2] = mem_info.totalswap;
    unsigned long ret = copy_to_user(_info, info, 3 * sizeof(unsigned long));
    kfree(info);
    return ret;
}