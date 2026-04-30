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

## Compilación y Uso

1. **Compilar todo**:
   ```bash
   make
   ```
2. **Ejecutar Benchmarks**:
   ```bash
   ./scripts/benchmark.py
   ```

## Diferencias Tecnológicas
- **opengl_es2**: Utiliza librerías de utilidad estándar y carga de archivos dinámica. Es ideal para aprender los conceptos básicos de GPGPU.
- **opengl_sc2**: Sigue estándares de seguridad crítica (Safety Critical). Todo está autocontenido (shaders en el binario) y el control de errores es exhaustivo.

sudo apt-get install libshaderc-dev libvulkan-dev libglfw3-dev
