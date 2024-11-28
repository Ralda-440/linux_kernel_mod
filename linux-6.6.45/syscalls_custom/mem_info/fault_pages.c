#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/swap.h>
#include <linux/zswap.h>
#include <linux/vmstat.h>
#include <linux/swapfile.h>
#include <linux/uaccess.h>

SYSCALL_DEFINE1(fault_pages, unsigned long __user *, _info){
    //Informacion de paginas de fallo
    unsigned long *info = kmalloc_array(NR_VM_EVENT_ITEMS, sizeof(unsigned long), GFP_KERNEL);
    if (!info)
    {
        return -ENOMEM;
    }
    all_vm_events(info);
    //Copiar a espacio de usuario
    unsigned long ret = copy_to_user(_info, &info[23], 2 * sizeof(unsigned long));
    kfree(info);
    return ret;
}