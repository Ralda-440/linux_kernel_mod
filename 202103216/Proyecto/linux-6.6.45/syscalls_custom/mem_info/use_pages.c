#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/swap.h>
#include <linux/zswap.h>
#include <linux/vmstat.h>
#include <linux/swapfile.h>
#include <linux/uaccess.h>

SYSCALL_DEFINE1(use_pages,unsigned long __user *, _info){
    //Obtener informacion de paginas
    unsigned long pages[NR_LRU_LISTS];
    for (int lru = LRU_BASE; lru < NR_LRU_LISTS; lru++)
		pages[lru] = global_node_page_state(NR_LRU_BASE + lru);
    //Informacion de paginas
    unsigned long *info = kmalloc_array(2, sizeof(unsigned long), GFP_KERNEL);
    if (!info)
    {
        return -ENOMEM;
    }
    //Paginas Activas
    info[0] = pages[LRU_ACTIVE_ANON] + pages[LRU_ACTIVE_FILE];
    //Paginas Inactivas
    info[1] = pages[LRU_INACTIVE_ANON] + pages[LRU_INACTIVE_FILE];
    //Copiar a espacio de usuario
    unsigned long ret = copy_to_user(_info, info, 2 * sizeof(unsigned long));
    kfree(info);
    return ret;
}