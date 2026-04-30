# Makefile Principal del Repositorio GPGPU

# Puedes pasar el tamaño por línea de comandos: make SIZE=2048
SIZE ?= 

SUBDIRS := 01_vector_add 02_mat_mult

.PHONY: all clean $(SUBDIRS)

all: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@ SIZE=$(SIZE)

clean:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done
