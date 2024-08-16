#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#define LOG_FILE_PATH "/var/log/kern.log"
#define LOG_LINES 5
#define BUFFER_SIZE 2048

SYSCALL_DEFINE1(lastkernlogs, char __user *, buffer)
{
    struct file *file;
    loff_t pos = 0;
    char *kernel_buffer;
    int size = 0;

    kernel_buffer = kzalloc(BUFFER_SIZE, GFP_KERNEL);
    if (!kernel_buffer)
        return -ENOMEM;

    file = filp_open(LOG_FILE_PATH, O_RDONLY, 0);
    if (IS_ERR(file)) {
        kfree(kernel_buffer);
        return PTR_ERR(file);
    }

    pos = vfs_llseek(file, -BUFFER_SIZE, SEEK_END);
    size = kernel_read(file, kernel_buffer, BUFFER_SIZE, &pos);

    filp_close(file, NULL);

    if (size >= 0) {
        int count_lines = 0;
        int pos_buffer = 0;
        for (int i = BUFFER_SIZE-1 ; i >= 0; i-- ){
            if (kernel_buffer[i] == '\n'){
                count_lines++;
                if (count_lines > LOG_LINES){
                    pos_buffer = i;
                    break;
                }
            }
        }
	    size = BUFFER_SIZE - (pos_buffer + 1);
        if (copy_to_user(buffer, &kernel_buffer[pos_buffer + 1], size)) {
            kfree(kernel_buffer);
            return -EFAULT;
        }
    }

    kfree(kernel_buffer);
    return size;
}
