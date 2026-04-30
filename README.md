# Repositorio de Tutoriales GPGPU

Este repositorio contiene una colección de tutoriales de GPGPU organizados por tipo de algoritmo, permitiendo comparar implementaciones en diferentes APIs.

## Estructura del Proyecto

### 01_vector_add (Suma de Vectores)
- **opengl_es2/**: Implementación estándar estilo OpenGL ES 2.0 (usa shaders externos y `esUtil`).
- **opengl_sc2/**: Implementación estricta OpenGL SC 2.0 (usa shaders embebidos y `scUtil`).
- **vulkan_headless/**: Implementación en Vulkan (Cálculo puro, sin ventana).
- **vulkan_gui/**: Implementación en Vulkan con visualización de ventana.

### 02_mat_mult (Multiplicación de Matrices)
- **opengl_es2/**: Implementación estándar estilo OpenGL ES 2.0.
- **opengl_sc2/**: Implementación estricta OpenGL SC 2.0 (shaders embebidos).
- **vulkan_headless/**: Implementación en Vulkan (Cálculo puro).
- **openmp/**: Implementación CPU multihilo para referencia.

## Requisitos del Sistema y Dependencias

Para compilar y ejecutar estos tutoriales, necesitarás un entorno Linux con soporte para desarrollo en C/C++, OpenGL y Vulkan.

### 1. Dependencias Base
Asegúrate de tener instaladas las herramientas esenciales de compilación y las librerías de desarrollo:

```bash
sudo apt-get update
sudo apt-get install build-essential python3
```

### 2. Dependencias de APIs Gráficas (OpenGL ES / SC / Vulkan)
Instala las librerías de desarrollo necesarias para cada API:

```bash
# OpenGL ES 2.0, EGL y X11
sudo apt-get install libgles2-mesa-dev libegl1-mesa-dev libx11-dev

# Vulkan y Compilación de Shaders (Shaderc)
sudo apt-get install libvulkan-dev libshaderc-dev libglfw3-dev
```

> **Nota sobre Vulkan**: Los Makefiles de Vulkan en este repositorio apuntan por defecto a una ruta específica del SDK (`~/vulkan/...`). Si tienes Vulkan instalado a través de los paquetes del sistema, es posible que necesites ajustar la variable `VULKAN_SDK_PATH` en los Makefiles correspondientes o simplemente eliminar las rutas específicas si las librerías ya están en el path del sistema.

### 3. OpenMP
Para la versión de CPU multihilo:
```bash
# Generalmente viene incluido con GCC
sudo apt-get install libomp-dev  # (Solo si usas Clang/LLVM)
```

### 4. Verificación del Entorno
Puedes verificar si tu sistema detecta correctamente las capacidades gráficas con:
```bash
# Verificar OpenGL
glxinfo | grep "OpenGL version"

# Verificar Vulkan
vulkaninfo | head -n 20
```

## Compilación y Uso

El proyecto está diseñado para ser flexible en su construcción:

1. **Compilar todo el repositorio**:
   Desde la raíz del proyecto, puedes compilar todas las versiones de todos los algoritmos a la vez:
   ```bash
   make
   ```

2. **Compilar una versión específica**:
   Si solo te interesa una API o un algoritmo concreto, cada carpeta es independiente y tiene su propio `Makefile`. Simplemente entra en la carpeta deseada y ejecuta `make`:
   ```bash
   cd 01_vector_add/vulkan_headless
   make
   ```

3. **Limpieza**:
   Para borrar los archivos binarios y temporales, puedes usar `make clean` tanto en la raíz (para limpiar todo) como dentro de cada carpeta específica.

4. **Ejecución**:
   Una vez compilado, para ejecutar un programa específico solo tienes que llamar al ejecutable generado:
   ```bash
   ./vector_add  # (O el nombre correspondiente al ejecutable)
   ```

5. **Benchmarks automáticos**:
   Para comparar todas las versiones rápidamente:
   ```bash
   ./scripts/benchmark.py --size 1024
   ```

## Diferencias Tecnológicas
- **opengl_es2**: Utiliza librerías de utilidad estándar y carga de archivos dinámica. Es ideal para aprender los conceptos básicos de GPGPU.
- **opengl_sc2**: Sigue estándares de seguridad crítica (Safety Critical). Todo está autocontenido (shaders en el binario) y el control de errores es exhaustivo.