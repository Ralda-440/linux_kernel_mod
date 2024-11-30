# Guía para Modificaciones al Kernel de Linux

Esta guía está diseñada para orientar a los desarrolladores en la creación e implementación de diversas modificaciones al kernel de Linux, explorando desde la personalización de syscalls hasta la integración de herramientas de monitoreo.

## Especificaciones Técnicas
* Versión del kernel: Se utiliza el kernel 6.6.45, descargado desde [kernel.org](https://kernel.org/).
* Sistema operativo: Linux Mint 22, descargado desde [Linux Mint](https://linuxmint.com/).

## Contenido de la Guía
1. **Compilación e instalación del kernel de Linux**:
Proceso detallado para descargar, configurar y compilar el kernel desde el código fuente.

2. **Modificaciones al kernel**:
Instrucciones para personalizar aspectos del kernel que indican al usuario que está corriendo una versión personalizada.

3. **Implementación de syscalls personalizadas**:
Creación de llamadas al sistema específicas para tareas avanzadas, con ejemplos prácticos.

4. **Dashboard Web para monitoreo**:
Guía para desarrollar un dashboard interactivo que muestra en tiempo real el uso de la memoria del sistema operativo.

5. **Resolución de problemas**:
Registro de los errores comunes encontrados durante el proceso y sus respectivas soluciones.

### [Vea la guía completa aquí](/docs/manual.md).

## Aplicación de las Modificaciones
Como parte de esta guía, se desarrolla una **API REST** en **C** utilizando el framework [**Ulfius HTTP**](https://github.com/babelouest/ulfius). Esta **API** interactúa directamente con las **syscalls** personalizadas para monitorear el **uso** de la **memoria** del sistema.

El resultado de esta implementación es consumido por un **Dashboard Web** construido en **React**, que presenta información visual mediante **gráficas** intuitivas. Estas gráficas facilitan la comprensión de la distribución de recursos de memoria, proporcionando un sistema robusto para el análisis en tiempo real.

## Objetivo de la Guía
Proveer un recurso completo y práctico para desarrolladores interesados en explorar las capacidades avanzadas del kernel de Linux, desde su compilación hasta la creación de herramientas personalizadas para monitoreo y análisis.

## Limitaciones y Decisión de Despliegue
El Dashboard Web no fue desplegado en la nube debido a las características del proyecto. Esto se debe a que la modificación del kernel del sistema operativo es un requisito esencial para su funcionamiento, y la mayoría de los servicios de máquinas virtuales en la nube no permiten realizar cambios directos en el kernel.

Por esta razón, el desarrollo y las pruebas se realizaron en un entorno local controlado, donde fue posible personalizar el kernel y realizar las integraciones necesarias con las syscalls implementadas.