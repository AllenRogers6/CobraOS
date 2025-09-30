CC = gcc
CFLAGS = -I arch/x86/drivers/screen/include -I lib/include -ffreestanding -O0 -Wall -Wextra -nostartfiles -nostdlib -m32 -g -fno-omit-frame-pointer
libDir = lib
buildDir = build

# Find all .c files in the lib directory
libSrc = $(wildcard $(libDir)/*.c)
# Map .c files to .o files in the build directory, preserving the lib subdirectory
OBJS = $(patsubst $(libDir)/%.c, $(buildDir)/$(libDir)/%.o, $(libSrc))

# Default target
all: $(OBJS)

# Create build directories before compiling
$(buildDir)/$(libDir)/%.o: $(libDir)/%.c
	@mkdir -p $(buildDir)/$(libDir)
	@echo "Compiling $<"
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build artifacts
clean:
	@echo "Cleaning up..."
	rm -rf $(buildDir)/*
