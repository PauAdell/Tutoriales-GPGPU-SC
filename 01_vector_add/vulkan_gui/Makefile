# Makefile para Suma de Vectores - Vulkan GUI

CXX := g++

# Define the SDK path (Update this to your actual folder name!)
VULKAN_SDK_PATH = $(HOME)/vulkan/1.4.341.1/x86_64

# Header and Library locations
VULKAN_CFLAGS := -I$(VULKAN_SDK_PATH)/include
VULKAN_LIBS   := -L$(VULKAN_SDK_PATH)/lib -lshaderc_combined -lvulkan -lglfw -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

# Flags Generales
CXXFLAGS := -std=c++17 -O2 -g

ifneq ($(SIZE),)
    CXXFLAGS += -DSIZE=$(SIZE)
endif

# Objetivo
TARGET := window_vector_add

.PHONY: all clean

all: $(TARGET)

$(TARGET): window_vector_add.cpp
	$(CXX) $(CXXFLAGS) $(VULKAN_CFLAGS) $< -o $@ $(VULKAN_LIBS)

clean:
	rm -f $(TARGET)