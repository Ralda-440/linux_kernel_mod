#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/swap.h>
#include <linux/zswap.h>
#include <linux/vmstat.h>
#include <linux/swapfile.h>
#include <linux/uaccess.h>

SYSCALL_DEFINE1(use_mem, unsigned long __user *, _info){
    struct sysinfo mem_info;
    si_meminfo(&mem_info);
    si_swapinfo(&mem_info);
    //Informacion de memoria
    unsigned long *info = kmalloc_array(4, sizeof(unsigned long), GFP_KERNEL);
    if (!info)
    {
        return -ENOMEM;
    }
    //Memoria libre en KB
    info[0] = mem_info.freeram << (PAGE_SHIFT - 10);
    //Memoria usada en KB
    unsigned long available = si_mem_available();
    info[1] = (mem_info.totalram - available) << (PAGE_SHIFT - 10);
    //Memoria cacheada en KB
    info[2] = (global_node_page_state(NR_FILE_PAGES) - total_swapcache_pages() - mem_info.bufferram) << (PAGE_SHIFT - 10);
    //Memoria total en KB
    info[3] = mem_info.totalram << (PAGE_SHIFT - 10);
    unsigned long ret = copy_to_user(_info, info, 4 * sizeof(unsigned long));
    kfree(info);
    return ret;
}