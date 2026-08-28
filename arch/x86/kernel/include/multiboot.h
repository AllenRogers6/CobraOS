#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#include <stdint.h>

#define MULTIBOOT_MEMORY_AVAILABLE 1
#define MULTIBOOT_MEMORY_RESERVED 2

typedef struct {
  uint32_t size;
  uint32_t base_addr_low;
  uint32_t base_addr_high;
  uint32_t length_low;
  uint32_t length_high;
  uint32_t type;
} __attribute__((packed)) multiboot_memory_map_t;

typedef struct {
  uint32_t flags;       // Multiboot info flags
  uint32_t mem_lower;   // Available memory from BIOS
  uint32_t mem_upper;   // Available memory from BIOS (high)
  uint32_t boot_device; // Boot device
  uint32_t cmdline;     // Command line
  uint32_t mods_count;  // Modules count
  uint32_t mods_addr;   // Modules address
  // ELF section headers
  uint32_t num;   // Number of ELF section headers
  uint32_t size;  // Size of each ELF section header
  uint32_t addr;  // Address of ELF section headers
  uint32_t shndx; // Section header string table index
  // Memory map (if flags bit 6 set)
  uint32_t mmap_length; // Length of memory map
  uint32_t mmap_addr;   // Address of memory map
  // Drives info
  uint32_t drives_length; // Size of drives info
  uint32_t drives_addr;   // Address of drives info
  // ROM configuration table
  uint32_t config_table; // ROM configuration table
  // Boot loader name
  uint32_t boot_loader_name; // Boot loader name
  // APM table
  uint32_t apm_table; // APM table
  // VBE info
  uint32_t vbe_control_info;
  uint32_t vbe_mode_info;
  uint16_t vbe_mode;
  uint16_t vbe_interface_seg;
  uint16_t vbe_interface_off;
  uint16_t vbe_interface_len;
  // Framebuffer info (if flags bit 12 set)
  uint64_t framebuffer_addr;
  uint32_t framebuffer_pitch;
  uint32_t framebuffer_width;
  uint32_t framebuffer_height;
  uint8_t framebuffer_bpp;
  uint8_t framebuffer_type;
  // ... color info follows, but we can ignore for now
} __attribute__((packed)) multiboot_info_t;

void parse_multiboot_info(uint32_t multiboot_info_ptr);
void print_memory_map(void);

#endif
