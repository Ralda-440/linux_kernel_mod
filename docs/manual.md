# Introducción al Kernel de Linux

* [Compilar e instalar el kernel de Linux](#compilar-e-instalar-el-kernel-de-linux)
* [Modificaciones al Kernel de Linux](#modificaciones-al-kernel-de-linux)
* [Implementar llamadas al sistema (syscalls) personalizadas](#implementar-llamadas-al-sistema-syscalls-personalizadas)
* [Dashboard Web para el monitoreo del uso de memoria del Sistema Operativo](#dashboard-web-para-el-monitoreo-del-uso-de-memoria-del-sistema-operativo) 
* [Problemas Encontrados](#problemas-encontrados)

## Compilar e instalar el kernel de Linux

Antes de realizar Modificaciones al kernel e implementar syscalls, es importante preparar el entorno donde se trabajara. Se instalaran herramientas para compilar el kernel y se descargara el kernel que servirá como base para las modificaciones e implementaciones del las syscalls.

* ### Instalar Herramientas y paquetes necesarios para la compilación.
    Estas herramientas y paquetes son de suma importancia para el kernel se compile correctamente. Si desea puede investigar para que funciona cada una. Las instalaremos con el siguiente comando

    ```bash
    sudo apt install build-essential libncurses-dev bison flex libssl-dev libelf-dev fakeroot dwarves
    ```

* ### Descargar el Kernel de Linux
    El Kernel de Linux se descargara desde la pagina oficial [kerne.org](https://www.kernel.org/). Descargue la versión mas reciente **longterm** disponible. En esta documentación se utilizara la versión 6.6.45 pero la manera de modificar el kernel e implementar las syscalls es el mismo.

    ![Texto alternativo](./imagenes/kernel.png)

    Para descargarlo compie el link de **tarball** y use el comando **wget**.

    ```bash
    wget https://cdn.kernel.org/pub/linux/kernel/v6.x/linux-6.6.46.tar.xz
    ```
    
    Ahora descomprímalo en el directorio donde quiera trabajar.

    ```bash
    tar -xvf linux-6.6.45.tar.xz
    ```

* ### Configurar el Kernel
    Primero se debe ingresar al directorio del código fuente
    ```bash
    cd linux-6.6.44
    ```

    La configuracion que usa el kernel al momento de compilarse esta dado por el archivo .config. Este archivo se tiene que crear. Para crear este archivo se puede utilizar 

    ```bash
    make menuconfig
    ```

    que mostrara una interfaz grafica en la que se puede seleccionar y ajustar varias opciones y caracteristicas del kernel.

    Sin embargo para simplificar el procedimiento, se utilizara el archivo .config que utilizo el kernel actual para compilarse y que se encuntra actualmente instalado.  

    Con el siguiente comando se copiara el archivos .config a el directorio donde usted se encuentra.

    ```bash
    cp -v /boot/config-$(uname -r) .config
    ```

    Sin embargo, este contiene tiene opciones, caracteristicas, modulos y drivers que no se usan en nuestro sistema operativo, ya que este esta hecho para compilar un kernel para diferentes tipos de hardware. Debido a eso, el proceso de compilacion tardaria demasiado, asi que lo que haremos sera modificarlo para que solo compile las opciones, caracteristicas, modulos y drivers que solo esta usando nuestro sistema operativo. Asi el proceso de compilacion sera mas rapido. Cabe decir que si usted desea usar el kernel modificado funcione correctamente en otras computadora, tendria que usar el archivo original o crear uno usando la herramienta anteriormente mencionada. Para modificarlos usaremos el siguiente comando

    ```bash
    make localmodconfig
    ```

    Otro problema es el que al añadir un nuevo modulo o archivos de codigo, estos no cuentan con una firma digital que confirman que son autenticos. Asi que para evitar errores desactivaremos las claves que se encuentran en el archivo .config que verifican esa informacion. Utilizamos el siguiente comando

    ```bash
    scripts/config --disable SYSTEM_TRUSTED_KEYS
    scripts/config --disable SYSTEM_REVOCATION_KEYS
    scripts/config --set-str CONFIG_SYSTEM_TRUSTED_KEYS ""
    scripts/config --set-str CONFIG_SYSTEM_REVOCATION_KEYS ""
    ```
    También se pude llegar a tener problemas con BPF/BTF. Se recomienda desactivarlo usando el siguiente comando:

    ```bash
    # Desactivar las opciones relacionadas con BPF
    scripts/config --disable CONFIG_BPF
    scripts/config --disable CONFIG_BPF_SYSCALL
    scripts/config --disable CONFIG_BPF_JIT
    scripts/config --disable CONFIG_BPF_JIT_ALWAYS_ON
    scripts/config --disable CONFIG_BPF_JIT_DEFAULT_ON

    # Desactivar las opciones relacionadas con BTF y debug
    scripts/config --disable CONFIG_PAHOLE_HAS_BTF_TAG
    scripts/config --disable CONFIG_DEBUG_INFO_BTF
    scripts/config --disable CONFIG_DEBUG_INFO_BTF_MODULES
    ```
* ### Compilar el Kernel

    Para compilar el kernel usaremos **fakeroot**. Utilizar **fakeroot** es necesario por que nos permite ejecutar el comando **make** en un entorno donde parece que se tiene permisos de superusuario para la manipulación de ficheros. Es necesario para permitir a este comando crear archivos (tar, ar, .deb etc.) con ficheros con permisos/propietarios de superusuario.

    ```bash
    fakeroot make
    ```

    Se puede acelerar la compilacion del kernel utilizando varios nucleos de la CPU con el siguiente comando

    ```bash
    fakeroot make -jN
    ```
    Donde **N/** es el numero de núcleos que desee usar.

    Al finalizar la compilacion utilizaremos el siguiente comando para ver si ocurrio algun error dunrante la compilacion. Si el resultando del comando es **0**, signfica que no hubo errores y podemos instalar el kernel. El comando es

    ```bash
    echo $?
    ```
    Si desea o requiere eliminar las configuraciones o archivos creados durante la configuración y compilación del kernel, puede usar los siguientes comandos. Tomar en cuenta que también se borrara el archivo .config que se creo anteriormente.

    ```bash
    make clean
    make mrproper
    ```


* ### Instalar el Kernel

    La instalacion se divide en dos partes
    * Instalar los módulos del Kernel
    * Instalar el Kernel mismo

    #### Instalar los módulos del Kernel

    Utilizaremos el siguiente comando

    ```bash
    sudo make modules_install
    ```

    #### Instalar el Kernel mismo

    Utilizaremos el siguiente comando

    ```bash
    sudo make install
    ```
    Por ultimo será necesario reiniciar nuestra computadora. 

    ```bash
    sudo reboot
    ```

* ### Observaciones
    Si el kernel que instalamos es de una versión anterior a nuestro kernel actual del sistema operativo, al iniciar siempre se eligiera automáticamente el que tenga la versión mayor. Para seleccionar la versión del kernel que queremos utilizar podemos acceder al GRUB. Para acceder al GRUB debe investigar que tecla utiliza su distribución y usarla en el arranque del sistema.


## Modificaciones al kernel de Linux
Ya que podemos compilar nuestro kernel, ahora aprenderemos a modificarlos. Las modificaciones que se explican en esta sección serán simples. No reflejan la gran cantidad y complejidad de modificaciones que se pueden realizar. Pero serviran para introducirnos en el kernel de nuestro sistema operativo. 

* ### Mensaje Personalizado
    Se imprimirá un mensaje personalizado que se guardara en los logs de nuestro kernel.

    Desde la raiz del codigo fuente abriremos el siguiente archivo 

    ```bash
    nano init/main.c
    ```

    Ya en el archivos, buscaremos **"void start_kernel"**. Esta es un metodo que se ejecuta al iniciar el kernel. Lo que haremos sera un agregar un **"printk"** al inicio del metodo, con cualquier mensaje que queramos. 

    ![Texto alternativo](./imagenes/mod1.png) 

    Este mensaje lo podremos ver con el siguiente comando

    ```bash
    dmesg
    ```

* ### Nombre del Kernel

    Modificar el nombre de nuestro Kernel.

    Desde la raiz del codigo fuente abriremos el siguiente archivo
    ```bash
    nano include/linux/uts.h
    ```

    Dentro del archivo podremos encontrar facilmente **"UTS_SYSNAME"**. Que es donde se almacena el nombre de nuestro Kernel. Podemos colocar cualquiera que queramos.

    ![Texto alternativo](./imagenes/mod2.png) 


## Implementar llamadas al sistema (syscalls) personalizadas
Ahora veremos como crear una nueva syscall e implementarla. También como usarla a nivel de usuario en un programa en lenguaje C. El procedimiento es el mismo para cualquier syscall, lo único que cambia es el código propio de la syscall. Teniendo eso en cuenta, se explicara detalladamente el proceso para la primera sycall y será el mismo para cualquiera.

Ya que el objetivo es: implementar una syscall, no se explicara a fondo el código propio de la syscall. 


* ### Syscall: devuelve la hora actual
    Esta syscall devuelve la hora actual en segundos desde el epoch linux.

    * #### Paso 1
        Crear una carpeta en el directorio del código fuente con el nombre que tendrá la syscall y moverse al directorio creado.

        ```bash
        mkdir currenttimeseg
        cd currenttimeseg
        ```
    * #### Paso 2

        Crear un archivo **.c** con el nombre de la syscall en el directorio actual.

        ```bash
        nano currenttimeseg.c
        ```

        Dentro de este archivo hira el codigo propio de la syscall.
        El codigo para la syscall es

        ```c
        #include <linux/kernel.h>
        #include <linux/syscalls.hc
        #include <linux/time.h>

        SYSCALL_DEFINE0(currenttimeseg)
        {
            struct timespec64 ts;
            ktime_get_real_ts64(&ts);
            return ts.tv_sec;
        }
        ```

        Se utiliza la funcion interna del kernel **"ktime_get_real_ts64"** para obtener la hora actual en segundos. Tambien un **struct** de tipo **timespec64** para guardarla y retornar **tv_sec**, que corresponde al tiempo en segundos.

    * #### Paso 3
        Crear un **Makefile** en el directorio actual.

        ```bash
        nano Makefile
        ```
        Tendrá la siguiente linea.

        ```
        obj-y := urrenttimeseg.o
        ```

        Esto es para garantizar que el archivo  **currenttimeseg.c** se compile e incluya en el codigo fuente del Kernel.

    * #### Paso 4
        Añadir el directorio **currenttimeseg/** al **Makefile** del Kernel.

        Primero volver al directorio del codigo fuente del kernel y abrir el archivo **Makefile**.

        ```bash
        cd ..
        nano Makefile
        ```

        Buscar **"core-y"** dentro del archivo. 
        Encontraremos la siguiente linea

        ![Texto alternativo](./imagenes/core.png)

        En esta linea agregaremos el directorio que creamos anteriormente: **currenttimeseg/**. Tiene que haber un espacio entre cada directorio que se agrega a la linea.
        
    * #### Paso 5
        Agregar la syscall a la tabla de syscalls del sistema.

        Si está en un sistema de 32 bits, deberá cambiar 'syscall_32.tbl'. Para 64 bits, cambie 'syscall_64.tbl'

        Abra el archivo con el siguiente comando

        ```bash
        nano arch/x86/entry/syscalls/syscall_64.tbl
        ```

        En la ultima linea del archivo agregar la siguiente linea

        ```
        548 64 currenttimeseg sys_currenttimeseg
        ```

        Tiene que ser el mismo nombre de la syscall. Guarde y salga del archivo.
    * #### Paso 6
        Agregar la syscall al archivo de encabezado de syscalls.

        Abra el archivo con el siguiente comando.

        ```bash
        nano include/linux/syscalls.h
        ```

        Añadir la siguiente linea al final del documento antes del **#endif**.

        ```c
        asmlinkage long sys_currenttimeseg(void);
        ```

        Guardar y salir de archivo. Estos son todos los paso para implentar una syscall al kernel. Ahora solo tiene que compilar el kernel.
        


* ### NOTA: Para las siguiente 2 syscall solo se indicara que se tendrá que colocar en cada archivo, ya que el procedimiento es el mismo. 

* ### Syscall: devuelve el tiempo de actividad del sistema
    Esta syscall devuelve el tiempo de actividad del sistema desde el ultimo reinicio.

    * #### Archivo **.c** con el nombre de la syscall.

        ```c
        #include <linux/kernel.h>
        #include <linux/syscalls.h>
        #include <linux/timekeeping.h>

        SYSCALL_DEFINE0(systemuptimeseg)
        {
            struct timespec64 uptime;
            ktime_get_boottime_ts64(&uptime);
            return uptime.tv_sec;
        }
        ```

    * #### Archivo **Makefile**
        ```bash
        obj-y := systemuptimeseg.o
        ```


    * #### Agregar al archivo **Makefile** del Kernel
        ![Texto alternativo](./imagenes/core.png)

    * #### Agregar la syscall a la tabla de syscalls del sistema
        ```
        549 64 systemuptimeseg sys_systemuptimeseg
        ```
    * #### Agregar la syscall al archivo de encabezado de syscalls.

        ```c
        asmlinkage long sys_systemuptimeseg(void);
        ```

* ### Syscall: Recupera los últimos mensajes del kernel
    Esta syscall recupera los últimos 5 mensajes del log del kernel

    * #### Archivo **.c** con el nombre de la syscall.

        ```c
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

        ```

    * #### Archivo **Makefile**
        ```bash
        obj-y := lastkernlogs.o
        ```


    * #### Agregar al archivo **Makefile** del Kernel
        ![Texto alternativo](./imagenes/core.png)

    * #### Agregar la syscall a la tabla de syscalls del sistema
        ```
        550 64 lastkernlogs sys_lastkernlogs
        ```
    * #### Agregar la syscall al archivo de encabezado de syscalls.

        ```c
        asmlinkage long sys_lastkernlogs(char __user *buffer);
        ```
* ### Syscalls: Encriptar y Desencriptar
    El metodo que se usa para encriptar y desencriptar es por XOR, lo que hace que las 2 syscalls utilizan la misma lógica para encriptar y desencriptar, y también reciben los mismo 4 parámetros (path_input, path_output, cantidad_threads, path_key), entonces se crea una función que reciba estos 4 parámetros y solo se invoca en las definiciones de las syscalls.

    ```c
    SYSCALL_DEFINE4(my_encrypt, char __user *, path_input, char __user *, path_output, int, no_threads, char __user *, path_key) {
        return function_syscall(path_input, path_output, no_threads, path_key);
    }

    SYSCALL_DEFINE4(my_decrypt, char __user *, path_input, char __user *, path_output, int, no_threads, char __user *, path_key) {
        return function_syscall(path_input, path_output, no_threads, path_key);
    }
    ```
    * Funcion 'function_syscall'
        
        Esta función es la encargada de recibir los parámetros de la syscall y encriptar o desencriptar dependiendo si el input esta o no esta encriptado. 

        ```c
        int function_syscall(char __user *path_input, char __user *path_output, int no_threads, char __user *path_key);
        ```
    * Implementación de la función 'function_syscall'

        1. Punteros del nivel de usuario
            
            Los punteros que se obtienen por lo parámetros apuntan a dirección a nivel de usuario, es decir, que no se pueden utilizar a nivel de kernel. La solución para esto es obtener el tamaño de las cadenas y realizar una copia. Para obtener el tamaño se utiliza "strnlen_user", que recibe como parámetros, un puntero de nivel de usuario y el tamaño máximo que leerá si no encuentra el final de la cadena(para evitar errores). También se utilizar "copy_from_user" para crear la copia a un puntero de nivel del kernel. 

            ```c
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
            ```
        2. División de fragmentos

            Debido a que hasta en tiempo de ejecucion se sabe en cuantos fragmentos se dividira la entrada, se utiliza un doble puntero **char \*\***, esto para crear en tiempo de ejecucion una lista de apuntadores y cada apuntador apunta a un fragmentos. La lectura del archivo input y calculo de los fragmentos se realiza con el siguientes codigo:

            ```c
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
            ```
        3. La lectura de la key

            Se hace desde el archivo donde se encuentra la llave hacia un puntero char * con el siguiente codigo:
            
            ```c
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
            ```
        4. Estructura para cada hilo

            Se creo una estructura con la informacion necesaria para que cada hilo pueda encriptar/desencriptar su fragmento. 

            ```c
            struct args_thread {
                struct completion *thread_completed;
                int no_thread;
                char *fragment_text;
                int size_fragment;
                char *key;
                int size_key;
            };
            ```
            Se utiliza la estructura **completion** definida por el kernel, que sirve para que el hilo principal espere a que los demás hilos terminen su tarea. El hilo es el encargado de enviar una señal al hilo principal a través de la estructura **completion**.

        5. Funcion para los hilos

            Con la informacion de la estructura **args_thread** es bastante sencillo encriptar/desencriptar. 
            
            ```c
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
            ```

        6. Espera del hilo principal

            El hilo principal necesita esperar a lo demas hilos para escribir el encriptado/desencriptado en el archivo de salida.

            Para la espera se utiliza la estructura completion.

            ```c
            for (int i = 0; i < no_threads; i++) {
                wait_for_completion(args_threads[i]->thread_completed);
                printk(KERN_INFO "Thread %d completed\n", i);
            }
            ```

        7. Escribir en el archivo de salida

            La escritura en el archivo es bastante simple una vez los hilos terminaron su tarea.

            ```c
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
            ```

        8. Liberacion de memoria

            Despues de escribir en el archivo de salida es necesario liberar toda la memoria utiliza para que pueda ser usada posteriormente por otros procesos.

            ```c
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
            ```
* ### Syscalls: Uso de memoria
    Esta syscall devuelve como esta distribuido el uso de la memoria física. Toda la información de la memoria se puede obtener utilizando la estructura **struct sysinfo** y usando 2 funciones para recuperar los datos y guardarlos en la estructura. La memoria libre, utilizada y cache se calcula de la siguiente forma:
    ```c
    struct sysinfo mem_info;
    si_meminfo(&mem_info);
    si_swapinfo(&mem_info);
    //Memoria libre en KB
    info[0] = mem_info.freeram << (PAGE_SHIFT - 10);
    //Memoria usada en KB
    unsigned long available = si_mem_available();
    info[1] = (mem_info.totalram - available) << (PAGE_SHIFT - 10);
    //Memoria cacheada en KB
    info[2] = (global_node_page_state(NR_FILE_PAGES) - total_swapcache_pages() - mem_info.bufferram) << (PAGE_SHIFT - 10);
    //Memoria total en KB
    info[3] = mem_info.totalram << (PAGE_SHIFT - 10);
    ```  
* ### Syscalls: Paginas de memoria de swap usadas y libre
    Esta llamada recupera cuenta de la memoria swap esta siendo utilizada y cuanta se encuentra libre. Toda la información se guarda en la estructura **struct sysinfo** utilizando 2 funciones para obtenerla. Se calculan de la siguiente forma:
    ```c
    struct sysinfo mem_info;
    si_meminfo(&mem_info);
    si_swapinfo(&mem_info);
    //Swap usado en paginas
    info[0] = mem_info.totalswap - mem_info.freeswap;
    //Swap libre en paginas
    info[1] = mem_info.freeswap;
    //Swap total en paginas
    info[2] = mem_info.totalswap;
    ``` 
* ### Syscalls: Cantidad de fallos de pagina
    Es syscall recupera la cantidad de fallos de paginas menores y mayores. Se utiliza un puntero de tipo **long** para guardar el array de toda la información relacionada de la memoria virtual. Se utiliza la función **all_vm_events** para recuperarla y guardarla en el puntero. Se obtiene de la siguiente forma:
    ```c
    unsigned long *info = kmalloc_array(NR_VM_EVENT_ITEMS, sizeof(unsigned long), GFP_KERNEL);
    all_vm_events(info);
    ```

    Los fallos menores se encuentran en la posicion 23 y los mayores en la posicion 24 del array.

* ### Syscalls: Paginas de memoria activas e inactivas
    Esta llamada recupera la cantidad de paginas que se encuentran en uso constante (Activas) y las que no se encuentran en uso o no se han usado en un determinado tiempo. Se utiliza la función **global_node_page_state()**, esta se encarga de recuperar varios datos relacionados con la memoria, incluyendo la memoria virtual.
    Se recupera de la siguiente forma:
    ```c
    //Obtener informacion de paginas
    unsigned long pages[NR_LRU_LISTS];
    for (int lru = LRU_BASE; lru < NR_LRU_LISTS; lru++)
		pages[lru] = global_node_page_state(NR_LRU_BASE + lru);
    //Paginas Activas
    info[0] = pages[LRU_ACTIVE_ANON] + pages[LRU_ACTIVE_FILE];
    //Paginas Inactivas
    info[1] = pages[LRU_INACTIVE_ANON] + pages[LRU_INACTIVE_FILE];
    ``` 
* ### Syscalls: Procesos que más memoria utilizan
    Esta syscall recupera los 5 proceso que mas utilizan memoria **física**. Se recorre toda la estructura de datos que guarda todos los procesos. Se utiliza un simple algoritmo que comprueba cuales son los mayores procesos que utilizan memoria y se guardan en un array de 5 elementos. Se utiliza una estructura personalizada para guardar los 5 procesos.
    ```c
    struct proc_info {
        char name[16];
        long pid;
        unsigned long pct_usage;
        unsigned long mm_rss;
    };

    struct proc_info *procs_info = kmalloc_array(top_procs, sizeof(struct proc_info), GFP_KERNEL);
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
    ```  
## Dashboard Web para el monitoreo del uso de memoria del Sistema Operativo

El Dashboard Web está compuesto por cinco gráficas que muestran en tiempo real el uso de la memoria del sistema. Esta aplicación, desarrollada con **React**, consume una **API REST** construida en **C** utilizando el framework [**Ulfius HTTP**](https://github.com/babelouest/ulfius).

Cada gráfica está asociada a un endpoint específico en la **API REST**. Estos **endpoints** interactúan con **syscalls** personalizadas, implementadas en el **kernel** del sistema, para obtener y procesar información detallada sobre el uso de la memoria.

### Gráficas

|Gráfica|Descripción|Endpoint|Syscalls|Representación Visual|
|-|-|-|-|-|
|Uso de Memoria Física (**gráfico de pie**)|Proporciona una descripción general de cómo se utiliza la memoria física, lo que facilita la comprensión de la distribución general de los recursos de memoria. La gráfica muestra: **memoria usada**, **memoria libre** y **memoria en caché**|Tipo **GET**. **/use_mem**|[Uso de Memoria](#syscalls-uso-de-memoria)|[Ver](/docs/imagenes/use_mem.png)|
|Uso de Memoria a lo largo del tiempo (**gráfico de líneas**)|Mostrará cómo el uso de la memoria evoluciona con el tiempo, con líneas separadas para la memoria física y swap, lo que permite a los usuarios identificar picos en el uso de la memoria o períodos de presión de la memoria.|Tipo **GET**. **/use_swap**|- [Uso de Memoria](#syscalls-uso-de-memoria)<br>- [Swap](#syscalls-paginas-de-memoria-de-swap-usadas-y-libre)|- [Ver Imagen 1](/docs/imagenes/use_physical.png)<br>- [Ver Imagen 2](/docs/imagenes/use_swap.png)|
|Fallos de páginas (**gráfico de barras**)|Muestra la cantidad de los fallos de página menores y fallos de página mayores|Tipo **GET**. **/fault_pages**|[Cantidad de Fallos de Páginas](#syscalls-cantidad-de-fallos-de-pagina)|- [Ver Imagen 1](/docs/imagenes/fault_menor.png)<br>- [Ver Imagen 2](/docs/imagenes/fault_major.png)|
|Páginas Activas e Inactivas (**gráfico de pie**)|Muestra las páginas de memoria activas e inactivas en el sistema. Proporciona una vista de cuánta memoria del sistema se utiliza activamente frente a la que está marcada como inactivo y potencialmente disponible|Tipo **GET**. **/use_pages**|[Páginas Activas e Inactivas](#syscalls-paginas-de-memoria-activas-e-inactivas)|[Ver](/docs/imagenes/pages_use.png)|
|Top 5 Procesos con Mayor Uso de Memoria (**gráfico de barras**)|Muestra los 5 procesos que consumen más memoria, mostrando el nombre del proceso, el PID y el porcentaje del uso de memoria. Permite a los usuarios identificar qué procesos consumen más memoria, lo que ayuda a diagnosticar procesos fuera de control.|Tipo **GET**. **/use_mem_top_procs**|[Procesos que más memoria utilizan](#syscalls-procesos-que-más-memoria-utilizan)|[Ver](/docs/imagenes/top.png)|


## Problemas Encontrados
1. Es importante que el nombre de nuestra syscall sea el mismo en todos los archivos en la que agregamos ya que si no fuera el mismo habrá errores de compilación.
    * Solución: siempre revisar con detenimiento si el nombre de nuestra funcion se encuentra bien escrito en todos los archivos.

2. No quitar las claves de firmas digitales del archivo .config ocasiona que no podamos agregar archivos nuevos y producirá errores en la compilación.
    * Solución: Verificar que hemos eliminar las clases de firmas digitales en el archivos **.config** o firmar digitalmente nuestro archivos de codigo. Para eliminar las claves utilizar 

    ```bash
    scripts/config --disable SYSTEM_TRUSTED_KEYS
    scripts/config --disable SYSTEM_REVOCATION_KEYS
    scripts/config --set-str CONFIG_SYSTEM_TRUSTED_KEYS ""
    scripts/config --set-str CONFIG_SYSTEM_REVOCATION_KEYS ""
    ```

3. Errores en la compilación debido a que no se puedan crear archivos (tar, ar, .deb etc.) con ficheros con permisos/propietarios de superusuario.

    * Solución: utilizar fakeroot para evitarlo ya que este crea un entorno donde parece que se tiene permisos de superusuario para la manipulación de ficheros.
    ```bash
    fakeroot make
    ```
4. Error al intentar utilizar los apuntadores de espacio de usuario en el espacio de kernel. 

    * Solución: Realizar una copia a apuntadores del espacio de kernel para manipular la información.
    
    ```c
    char *path_input_ = kmalloc(length, GFP_KERNEL);
    if (copy_from_user(path_input_, path_input, length)) {
        return -EFAULT;
    }
    ```
5. Problemas al leer y copiar los datos del archivo a la memoria. 

    *Solucion: Utilizar la funcion kernel_read, ya que esta cuenta con privilegios para copiar a espacio de kernel.

    ```c
    kernel_read(file_key, key, size_key, &pos);
    ```

6. Problemas con el BPF/BTF al compilar el kernel. Se puede llevar a tener problemas al usar el archivo .config del kernel actualmente instalada. 

    * Solución: Desactivarlo usando

    ```bash
    # Desactivar las opciones relacionadas con BPF
    scripts/config --disable CONFIG_BPF
    scripts/config --disable CONFIG_BPF_SYSCALL
    scripts/config --disable CONFIG_BPF_JIT
    scripts/config --disable CONFIG_BPF_JIT_ALWAYS_ON
    scripts/config --disable CONFIG_BPF_JIT_DEFAULT_ON

    # Desactivar las opciones relacionadas con BTF y debug
    scripts/config --disable CONFIG_PAHOLE_HAS_BTF_TAG
    scripts/config --disable CONFIG_DEBUG_INFO_BTF
    scripts/config --disable CONFIG_DEBUG_INFO_BTF_MODULES
    ```
7. Problemas con los CORS en la api en c. El autor del framework recomendó configurar los CORS para cada endpoint ya que configurarlos para todos es una tarea muy compleja.

    * Solución: Configurar CORS para cada endpoint
    ```c
    // Configure CORS
    u_map_put(response->map_header, "Access-Control-Allow-Origin", "*");
    ```
8. Problemas al interpretar el valor de la estructura **struct sysinfo** para obtener la información sobre la memoria. Los valores que maneja la estructura esta dado en **paginas** y para convertirlos a KB hay que multiplicarlo por 4 ya que cada pagina es un tamaño de 4KB. 
    * Solución: Desplazar 2 bit a la izquierda o multiplicarlos por 4
    ```c
    //Memoria libre en KB
    info[0] = mem_info.freeram << (PAGE_SHIFT - 10);
    ```
9. Tipo de dato incorrecto la crear el JSON para el response de la request en la api de c. En la función para crear un objeto JSON se debe indicar que tipo de dato se espera para convertirlo. Se cometió el error de indicar que se esperaba un dato de tipo float pero el dato era Integer, entonces al momento de convertirlo hubo un error de segmento de memoria porque el dato espera es mas grande que Integer.
    * Solución: parsear el dato de Integer a double para evitar el error de segmento
    ```c
    json = json_pack("{s:f, s:f, s:f, s:f}", "free", ((double)_info[0])/1024,
         "used", ((double)_info[1])/1024, "cached", ((double)_info[2])/1024, "total", ((double)_info[3])/1024);
    ``` 