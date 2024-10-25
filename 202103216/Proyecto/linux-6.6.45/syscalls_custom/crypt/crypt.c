#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>
#include <linux/kthread.h>
#include <linux/completion.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/string.h>

struct args_thread {
    struct completion *thread_completed;
    int no_thread;
    char *fragment_text;
    int size_fragment;
    char *key;
    int size_key;
};

int function_thread(void* arg) {
    struct args_thread *args = (struct args_thread *)arg;
    printk(KERN_INFO "Thread created %d\n", args->no_thread);
    //**********************************************************************
    //Encriptar fragmento por XOR con la clave
    for (int i = 0; i < args->size_fragment; i++) {
        args->fragment_text[i] = args->fragment_text[i] ^ args->key[i % args->size_key];
    }
    //**********************************************************************
    complete(args->thread_completed);  // Marca la tarea como completada
    return 0;
}

int function_syscall(char __user *path_input, char __user *path_output, int no_threads, char __user *path_key){
    int ret = 0;
    //**********************************************************************
    printk("Inicializando******************\n");
    int max_length = 1000;
    //Path input
    long length = strnlen_user(path_input, max_length);
    if (length == 0 || length > max_length) 
    {
        printk("Error: Invalid path_input\n");
        return -EINVAL;
    }
    char *path_input_ = kmalloc(length, GFP_KERNEL);
    if (copy_from_user(path_input_, path_input, length)) {
        return -EFAULT;
    }
    //Path output
    length = strnlen_user(path_output, max_length);
    if (length == 0 || length > max_length) 
    {
        printk("Error: Invalid path_output\n");
        return -EINVAL;
    }
    char *path_output_ = kmalloc(length, GFP_KERNEL);
    if (copy_from_user(path_output_, path_output, length)) {
        return -EFAULT;
    }
    //Path key
    length = strnlen_user(path_key, max_length);
    if (length == 0 || length > max_length) 
    {
        printk("Error: Invalid path_key\n");
        return -EINVAL;
    }
    char *path_key_ = kmalloc(length, GFP_KERNEL);
    if (copy_from_user(path_key_, path_key, length)) {
        return -EFAULT;
    }
    //**********************************************************************
    struct file *file_input;
    file_input = filp_open(path_input_, O_RDONLY, 0);
    if (IS_ERR(file_input)) {
        printk(KERN_ERR "Error opening input file\n");
        return PTR_ERR(file_input);
    }
    loff_t size_file = vfs_llseek(file_input,0,SEEK_END);
    loff_t pos = vfs_llseek(file_input, 0, SEEK_SET);
    printk(KERN_INFO "File size: %lld\n", size_file);
    
    int size_fragment = size_file / no_threads;
    int size_last_fragment = size_file % no_threads;

    printk(KERN_INFO "Fragment size: %d\n", size_fragment);
    printk(KERN_INFO "Last fragment size: %d\n", size_last_fragment+size_fragment);

    char **fragments_text = kmalloc_array(no_threads, sizeof(char *), GFP_KERNEL);

    if (!fragments_text) {
        ret = -ENOMEM;
        goto final;
    }

    for (int i = 0; i < no_threads; i++) {
        int size = size_fragment;
        if (i == (no_threads - 1)) {
            size = size_fragment + size_last_fragment;
        }
        fragments_text[i] = kmalloc_array(size, sizeof(char), GFP_KERNEL);
        if (!fragments_text[i]) {
            ret = -ENOMEM;
            goto final;
        }
    }

    for (int i = 0; i < no_threads; i++) {
        int size = size_fragment;
        if (i == (no_threads - 1)) {
            size = size_fragment + size_last_fragment;
        }
        kernel_read(file_input, fragments_text[i], size, &pos);
    }
    filp_close(file_input, NULL);

    printk(KERN_INFO "File Input read\n");

    //**********************************************************************

    struct file *file_key;
    file_key = filp_open(path_key_, O_RDONLY, 0);
    if (IS_ERR(file_key)) {
        printk(KERN_ERR "Error opening key file\n");
        return PTR_ERR(file_key);
    }
    loff_t size_key = vfs_llseek(file_key,0,SEEK_END);
    pos = vfs_llseek(file_key, 0, SEEK_SET);
    printk(KERN_INFO "Key size: %lld\n", size_key);
    char *key = kmalloc_array(size_key, sizeof(char), GFP_KERNEL);
    if (!key) {
        ret = -ENOMEM;
        goto final;
    }
    kernel_read(file_key, key, size_key, &pos);
    filp_close(file_key, NULL);

    printk(KERN_INFO "File Key read\n");
    //**********************************************************************

    struct task_struct **threads;
    struct args_thread **args_threads;

    // Asignación de memoria para los threads y argumentos
    threads = kmalloc_array(no_threads, sizeof(struct task_struct *), GFP_KERNEL);
    if (!threads)
        return -ENOMEM;  // Error si kmalloc falla

    args_threads = kmalloc_array(no_threads, sizeof(struct args_thread *), GFP_KERNEL);
    if (!args_threads) {
        kfree(threads);
        return -ENOMEM;  // Error si kmalloc falla
    }

    // Creación de threads
    for (int i = 0; i < no_threads; i++) {
        args_threads[i] = kmalloc(sizeof(struct args_thread), GFP_KERNEL);
        if (!args_threads[i]) {
            ret = -ENOMEM;
            goto cleanup_partial;
        }

        args_threads[i]->thread_completed = kmalloc(sizeof(struct completion), GFP_KERNEL);
        if (!args_threads[i]->thread_completed) {
            kfree(args_threads[i]);
            ret = -ENOMEM;
            goto cleanup_partial;
        }

        init_completion(args_threads[i]->thread_completed);
        args_threads[i]->no_thread = i;
        args_threads[i]->fragment_text = fragments_text[i];
        args_threads[i]->size_fragment = size_fragment;
        args_threads[i]->key = key;
        args_threads[i]->size_key = size_key;
        if (i == (no_threads - 1)) {
            args_threads[i]->size_fragment += size_last_fragment;
        }

        threads[i] = kthread_run(function_thread, args_threads[i], "encrypt_thread_no_%d", i);
        if (IS_ERR(threads[i])) {
            printk(KERN_ERR "Error creating thread %d\n", i);
            ret = PTR_ERR(threads[i]);
            goto cleanup_partial;
        }
    }

    // Esperar a que todos los threads terminen
    for (int i = 0; i < no_threads; i++) {
        wait_for_completion(args_threads[i]->thread_completed);
        printk(KERN_INFO "Thread %d completed\n", i);
    }

    printk(KERN_INFO "All threads completed\n");
    
    //Escribir en el archivo de salida

    struct file *file_output;
    file_output = filp_open(path_output_, O_WRONLY | O_CREAT, 0777);
    if (IS_ERR(file_output)) {
        printk(KERN_ERR "Error opening output file\n");
        return PTR_ERR(file_output);
    }
    pos = vfs_llseek(file_output, 0, SEEK_SET);
    for (int i = 0; i < no_threads; i++) {
        kernel_write(file_output, fragments_text[i], args_threads[i]->size_fragment, &pos);
    }
    filp_close(file_output, NULL);

    printk(KERN_INFO "File written\n");

cleanup_partial:
    // Liberar memoria en caso de error o al finalizar
    for (int i = 0; i < no_threads; i++) {
        if (args_threads[i]) {
            kfree(args_threads[i]->thread_completed);
            kfree(args_threads[i]->fragment_text);
            kfree(args_threads[i]);
        }
    }
    kfree(threads);
    kfree(args_threads);
final:
    kfree(path_input_);
    kfree(path_output_);
    kfree(path_key_);
    for (int i = 0; i < no_threads; i++) {
        if (fragments_text){
            kfree(fragments_text[i]);
        }
    }
    kfree(fragments_text);
    kfree(key);

    printk(KERN_INFO "Finalizando******************\n");

    return ret;
}

SYSCALL_DEFINE4(my_encrypt, char __user *, path_input, char __user *, path_output, int, no_threads, char __user *, path_key) {
    return function_syscall(path_input, path_output, no_threads, path_key);
}

SYSCALL_DEFINE4(my_decrypt, char __user *, path_input, char __user *, path_output, int, no_threads, char __user *, path_key) {
    return function_syscall(path_input, path_output, no_threads, path_key);
}
