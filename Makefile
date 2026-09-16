GIT_DESCRIBE := $(shell git describe --tags --dirty 2>/dev/null)
VERSION      ?= $(if $(GIT_DESCRIBE),$(GIT_DESCRIBE),0.0.0-unknown)
ARCH        ?= x86

ARCH_DIR    := arch/$(ARCH)
ARCH_CONFIG := $(ARCH_DIR)/config.mk

ifeq ($(wildcard $(ARCH_CONFIG)),)
$(error No config for ARCH=$(ARCH) (expected $(ARCH_CONFIG)))
endif

include $(ARCH_CONFIG)

ISO_PATH    := $(ARCH_DIR)/$(ISO_NAME)
SUB_FLAGS   := VERSION=$(VERSION) $(if $(VERBOSE),VERBOSE=1)

.PHONY: all
all:
	$(MAKE) -C $(ARCH_DIR) $(SUB_FLAGS)

.PHONY: run
run: all
	$(QEMU) $(QEMU_RUN_ARG) $(ISO_PATH) $(QEMU_FLAGS)

.PHONY: run-kernel
run-kernel: all
	$(QEMU) $(QEMU_KERNEL_ARG) $(KERNEL_ELF) $(QEMU_FLAGS)

.PHONY: debug
debug: all
	$(QEMU) $(QEMU_RUN_ARG) $(ISO_PATH) $(QEMU_FLAGS) -s -S &
	@echo "Attach: $(GDB) -ex 'set architecture $(GDB_ARCH)' -ex 'target remote :1234' $(KERNEL_ELF)"

.PHONY: clean
clean:
	$(MAKE) -C $(ARCH_DIR) clean

.PHONY: rebuild
rebuild: clean all

.PHONY: help
help:
	@echo "Usage: make [TARGET] [ARCH=<arch>] [VERSION=<ver>]"
	@echo "Current ARCH: $(ARCH)"
	@echo "QEMU:         $(QEMU)"
	@echo "ISO:          $(ISO_PATH)"
	@echo ""
	@echo "Targets: all run run-kernel debug clean rebuild"
	@ls -1 arch/*/config.mk 2>/dev/null | sed 's|arch/||;s|/config.mk||' \
		| xargs -I{} echo "  ARCH={} available"
