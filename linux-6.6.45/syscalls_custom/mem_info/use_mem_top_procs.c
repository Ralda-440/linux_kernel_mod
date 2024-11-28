#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/swap.h>
#include <linux/zswap.h>
#include <linux/vmstat.h>
#include <linux/swapfile.h>
#include <linux/uaccess.h>
#include <linux/sched.h>

SYSCALL_DEFINE1(use_mem_top_procs, void __user *, _info){
    #define top_procs 5

    struct sysinfo mem_info;
    si_meminfo(&mem_info);
    si_swapinfo(&mem_info);

    struct proc_info {
        char name[16];
        long pid;
        unsigned long pct_usage;
        unsigned long mm_rss;
    };

    struct proc_info *procs_info = kmalloc_array(top_procs, sizeof(struct proc_info), GFP_KERNEL);
    if (!procs_info)
    {
        return -ENOMEM;
    }
    //Inicializar mm_rss
    for (int i = 0; i < top_procs; i++)
    {
        procs_info[i].mm_rss = 0;
    }
    //Obtener informacion de procesos
    struct task_struct *task;
    unsigned long mm_rss = 0;
    for_each_process(task){
        if (!task->mm)
        {
            continue;
        }
        mm_rss = get_mm_rss(task->mm);
        for (int i = 0; i < top_procs; i++)
        {
            if (mm_rss > procs_info[i].mm_rss)
            {
                //Correr hacia abajo
                for (int j = top_procs - 1; j > i; j--)
                {
                    procs_info[j] = procs_info[j - 1];
                }
                //Insertar nuevo proceso
                procs_info[i].pid = task->pid;
                strncpy(procs_info[i].name, task->comm, 16);
                procs_info[i].pct_usage = (mm_rss * 1000) / mem_info.totalram;
                procs_info[i].mm_rss = mm_rss;
                break;
            }
        }
    }
    //Copiar a espacio de usuario
    unsigned long ret = copy_to_user(_info, procs_info, top_procs * sizeof(struct proc_info));
    kfree(procs_info);
    return ret;
}