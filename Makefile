CC = i686-elf-gcc
CFLAGS = -I libc/include -ffreestanding -O2 -Wall -Wextra -nostartfiles -nostdlib
libcDir = libc
buildDir = build

# Find all .c files in the libc directory
libcSrc = $(wildcard $(libcDir)/*.c)
# Map .c files to .o files in the build directory, preserving the libc subdirectory
OBJS = $(patsubst $(libcDir)/%.c, $(buildDir)/$(libcDir)/%.o, $(libcSrc))

# Default target
all: $(OBJS)

# Create build directories before compiling
$(buildDir)/$(libcDir)/%.o: $(libcDir)/%.c
	@mkdir -p $(buildDir)/$(libcDir)
	@echo "Compiling $<"
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build artifacts
clean:
	@echo "Cleaning up..."
	rm -rf $(buildDir)/*
