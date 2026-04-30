# Makefile para Multiplicación de Matrices - OpenMP

CC := gcc

# Flags Generales
CFLAGS   := -Ofast -ftree-vectorize -funroll-all-loops -g -fopenmp

ifneq ($(SIZE),)
    CFLAGS   += -DSIZE=$(SIZE)
endif

# Objetivo
TARGET := mat_mult

.PHONY: all clean

all: $(TARGET)

$(TARGET): mat_mult.c
	$(CC) $(CFLAGS) $< -o $@ -lm

clean:
	rm -f $(TARGET)
