# SPDX-License-Identifier: GPL-2.0
# Makefile for simple sched_ext scheduler

CLANG ?= clang
LLC ?= llc
CC ?= gcc

# BPF compilation flags
BPF_CFLAGS := -g -O2 -target bpf -D__TARGET_ARCH_x86
BPF_CFLAGS += -I/usr/include/bpf
BPF_CFLAGS += -I/usr/include

# Userspace compilation flags
CFLAGS := -g -O2 -Wall
LDFLAGS := -lbpf -lelf -lz

# Targets
BPF_OBJ := simple_scheduler.bpf.o
USER_BIN := simple_scheduler

.PHONY: all clean

all: $(USER_BIN)

# Compile BPF program
$(BPF_OBJ): simple_scheduler.bpf.c
	$(CLANG) $(BPF_CFLAGS) -c simple_scheduler.bpf.c -o $(BPF_OBJ)

# Compile userspace loader
$(USER_BIN): simple_scheduler.c $(BPF_OBJ)
	$(CC) $(CFLAGS) simple_scheduler.c -o $(USER_BIN) $(LDFLAGS)

clean:
	rm -f $(BPF_OBJ) $(USER_BIN)

install: all
	@echo "To run the scheduler, execute: sudo ./$(USER_BIN)"

help:
	@echo "Simple sched_ext scheduler build system"
	@echo ""
	@echo "Targets:"
	@echo "  all     - Build the scheduler (default)"
	@echo "  clean   - Remove built files"
	@echo "  install - Show installation instructions"
	@echo "  help    - Show this help message"
	@echo ""
	@echo "Requirements:"
	@echo "  - Linux kernel with CONFIG_SCHED_CLASS_EXT enabled"
	@echo "  - clang/llvm for BPF compilation"
	@echo "  - libbpf development files"
	@echo ""
	@echo "Usage:"
	@echo "  make          # Build the scheduler"
	@echo "  sudo ./simple_scheduler  # Run the scheduler"