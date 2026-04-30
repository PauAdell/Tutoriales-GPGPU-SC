# Makefile para Suma de Vectores - OpenGL SC 2.0 Strict

CC  := gcc

# Rutas Comunes OpenGL
COMMON_DIR := ../../common/opengl_sc2
INCDIR      := -I$(COMMON_DIR) -I$(COMMON_DIR)/GLSC2 -I.
LIBS        := -lGLESv2 -lEGL -lm -lX11 -lGLSC2

SC_COMMON_SRC := $(COMMON_DIR)/scShader.c $(COMMON_DIR)/esTransform.c $(COMMON_DIR)/esShapes.c $(COMMON_DIR)/scUtil.c

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
	$(CC) $(CFLAGS) $(SC_COMMON_SRC) $< -o $@ $(INCDIR) $(LIBS)

clean:
	rm -f $(TARGET)
