# Makefile para Suma de Vectores - OpenGL ES 2.0 Style

CC  := gcc

# Rutas Comunes OpenGL
COMMON_DIR := ../../common/opengl_sc2
INCDIR      := -I$(COMMON_DIR) -I$(COMMON_DIR)/GLSC2 -I.
LIBS        := -lGLESv2 -lEGL -lm -lX11 -lGLSC2

COMMON_SRC  := $(COMMON_DIR)/esShader.c $(COMMON_DIR)/esTransform.c $(COMMON_DIR)/esShapes.c $(COMMON_DIR)/esUtil.c

# Flags Generales
CFLAGS   := -Ofast -ftree-vectorize -funroll-all-loops -g

ifneq ($(SIZE),)
    CFLAGS   += -DSIZE=$(SIZE)
endif

# Objetivo
TARGET := vector_add

.PHONY: all clean

all: $(TARGET)

$(TARGET): vector_add.c
	$(CC) $(CFLAGS) $(COMMON_SRC) $< -o $@ $(INCDIR) $(LIBS)

clean:
	rm -f $(TARGET)
