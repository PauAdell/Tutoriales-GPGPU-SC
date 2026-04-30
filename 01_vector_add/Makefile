# Makefile Principal para Suma de Vectores

# Puedes pasar el tamaño por línea de comandos: make SIZE=2048
SIZE ?= 

SUBDIRS := opengl_es2 opengl_sc2 vulkan_gui vulkan_headless

.PHONY: all clean $(SUBDIRS)

all: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@ SIZE=$(SIZE)

clean:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done