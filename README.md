
# fluffy-sched - Simple sched_ext Scheduler

A minimal external scheduler for Linux using the sched_ext (Extensible Scheduler) framework.

## Overview

This is a bare-minimum implementation of a BPF-based scheduler using Linux's sched_ext framework. It implements a simple FIFO (First-In-First-Out) scheduling policy where tasks are enqueued to a global queue and dispatched in order.

## Features

- **Simple FIFO scheduling**: Tasks are scheduled in the order they arrive
- **Global dispatch queue**: All tasks share a single global queue
- **Minimal overhead**: Bare-bones implementation with minimal logic
- **BPF-based**: Runs in kernel space using eBPF for safety and performance

## Requirements

### Kernel Requirements
- Linux kernel 6.6 or later with `CONFIG_SCHED_CLASS_EXT=y`
- Check if your kernel supports sched_ext:
  ```bash
  grep CONFIG_SCHED_CLASS_EXT /boot/config-$(uname -r)
  ```

### Build Requirements
- `clang` and `llvm` (version 11 or later)
- `libbpf` development files
- `libelf` development files

### Installation on Ubuntu/Debian
```bash
sudo apt-get install clang llvm libbpf-dev libelf-dev
```

### Installation on Fedora/RHEL
```bash
sudo dnf install clang llvm libbpf-devel elfutils-libelf-devel
```

### Installation on Arch Linux
```bash
sudo pacman -S clang llvm libbpf libelf
```

## Building

Simply run:
```bash
make
```

This will:
1. Compile the BPF scheduler (`simple_scheduler.bpf.c`) to `simple_scheduler.bpf.o`
2. Compile the userspace loader (`simple_scheduler.c`) to `simple_scheduler`

To clean build artifacts:
```bash
make clean
```

## Running

The scheduler must be run with root privileges:

```bash
sudo ./simple_scheduler
```

The scheduler will:
1. Load the BPF program into the kernel
2. Attach it as the active scheduler
3. Run until you press Ctrl+C
4. Automatically detach when exiting

### Output
```
Successfully attached simple scheduler
Simple scheduler is running. Press Ctrl+C to exit.
```

Press `Ctrl+C` to stop the scheduler and return to the default Linux scheduler.

## How It Works

### Architecture

```
┌─────────────────────────────────────┐
│   Userspace (simple_scheduler.c)    │
│   - Loads BPF object                │
│   - Attaches struct_ops              │
│   - Keeps scheduler running          │
└──────────────┬──────────────────────┘
               │
               │ BPF syscalls
               │
┌──────────────▼──────────────────────┐
│   Kernel Space (BPF Program)        │
│   - simple_enqueue()                │
│   - simple_dispatch()               │
│   - simple_running()                │
│   - simple_stopping()               │
└─────────────────────────────────────┘
```

### Scheduling Operations

1. **Enqueue** (`simple_enqueue`): When a task becomes runnable, it's added to the global FIFO queue and dispatched to the global dispatch queue (DSQ).

2. **Dispatch** (`simple_dispatch`): When a CPU needs a task to run, it consumes from the global DSQ in FIFO order.

3. **Running** (`simple_running`): Called when a task starts executing (no special action in this minimal implementation).

4. **Stopping** (`simple_stopping`): Called when a task stops executing (no special action in this minimal implementation).

## Files

- `simple_scheduler.bpf.c` - BPF scheduler implementation
- `scx_common.bpf.h` - Common definitions and helpers for sched_ext
- `simple_scheduler.c` - Userspace loader program
- `Makefile` - Build system
- `README.md` - This file

## Limitations

This is a **minimal** scheduler for educational purposes. It:
- Uses a simple FIFO policy (no priorities, no fairness)
- Has no CPU affinity handling
- Has no load balancing
- Uses default time slices
- Has minimal error handling

For production use, consider more sophisticated schedulers like:
- `scx_simple` - Simple but more complete scheduler
- `scx_rusty` - Rust-based scheduler with better features
- `scx_lavd` - Latency-aware scheduler

## Troubleshooting

### "Failed to open BPF object"
- Ensure the BPF object file exists: `ls -l simple_scheduler.bpf.o`
- Rebuild with `make clean && make`

### "Failed to load BPF object"
- Check kernel version: `uname -r`
- Verify sched_ext support: `grep CONFIG_SCHED_CLASS_EXT /boot/config-$(uname -r)`
- Check dmesg for kernel errors: `sudo dmesg | tail`

### "Failed to attach struct_ops"
- Ensure you're running as root: `sudo ./simple_scheduler`
- Check if another sched_ext scheduler is running
- Verify BPF is enabled: `ls /sys/fs/bpf/`

### Compilation errors
- Ensure clang/llvm is installed: `clang --version`
- Ensure libbpf is installed: `pkg-config --modversion libbpf`
- Check include paths in Makefile

## Monitoring

While the scheduler is running, you can monitor its behavior:

```bash
# Check which scheduler is active
cat /sys/kernel/sched_ext/state

# View BPF program logs
sudo cat /sys/kernel/debug/tracing/trace_pipe

# Monitor system performance
top
htop
```

## References

- [Linux sched_ext documentation](https://docs.kernel.org/scheduler/sched-ext.html)
- [sched_ext GitHub repository](https://github.com/sched-ext/scx)
- [BPF documentation](https://docs.kernel.org/bpf/)

## License

GPL-2.0

## Contributing

This is a minimal example. For a more complete implementation, refer to the official sched_ext schedulers in the Linux kernel tree.
