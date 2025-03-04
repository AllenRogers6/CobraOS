CC = i686-elf-gcc
CFLAGS = -I arch/x86/drivers/screen/include -I lib/include -ffreestanding -O2 -Wall -Wextra -nostartfiles -nostdlib 
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
