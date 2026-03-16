/* SPDX-License-Identifier: GPL-2.0 */
#include "/usr/src/kernels/6.18.13-200.fc43.x86_64/vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>

char _license[] SEC("license") = "GPL";

#define SCX_SLICE_DFL (20ULL * 1000000)
#define SCX_DSQ_GLOBAL 0

#define BPF_STRUCT_OPS(name, args...) \
SEC("struct_ops/"#name) \
BPF_PROG(name, ##args)

#define BPF_STRUCT_OPS_SLEEPABLE(name, args...) \
SEC("struct_ops.s/"#name) \
BPF_PROG(name, ##args)

s32 BPF_STRUCT_OPS(simple_select_cpu, struct task_struct *p, s32 prev_cpu, u64 wake_flags)
{
	return prev_cpu;
}

void BPF_STRUCT_OPS(simple_enqueue, struct task_struct *p, u64 enq_flags)
{
	scx_bpf_dsq_insert(p, SCX_DSQ_GLOBAL, SCX_SLICE_DFL, 0);
}

void BPF_STRUCT_OPS(simple_dequeue, struct task_struct *p, u64 deq_flags) {}

void BPF_STRUCT_OPS(simple_dispatch, s32 cpu, struct task_struct *prev)
{
	scx_bpf_dsq_move_to_local(SCX_DSQ_GLOBAL);
}

void BPF_STRUCT_OPS(simple_tick, struct task_struct *p) {}

void BPF_STRUCT_OPS(simple_runnable, struct task_struct *p, u64 enq_flags) {}

void BPF_STRUCT_OPS(simple_running, struct task_struct *p) {}

void BPF_STRUCT_OPS(simple_stopping, struct task_struct *p, bool runnable) {}

void BPF_STRUCT_OPS(simple_quiescent, struct task_struct *p, u64 deq_flags) {}

bool BPF_STRUCT_OPS(simple_yield, struct task_struct *from, struct task_struct *to)
{
	return false;
}

void BPF_STRUCT_OPS(simple_set_weight, struct task_struct *p, u32 weight) {}

void BPF_STRUCT_OPS(simple_set_cpumask, struct task_struct *p, const struct cpumask *mask) {}

void BPF_STRUCT_OPS(simple_update_idle, s32 cpu, bool idle) {}

void BPF_STRUCT_OPS(simple_cpu_acquire, s32 cpu, struct scx_cpu_acquire_args *args) {}

void BPF_STRUCT_OPS(simple_cpu_release, s32 cpu, struct scx_cpu_release_args *args) {}

s32 BPF_STRUCT_OPS(simple_init_task, struct task_struct *p, struct scx_init_task_args *args)
{
	return 0;
}

void BPF_STRUCT_OPS(simple_exit_task, struct task_struct *p, struct scx_exit_task_args *args) {}

void BPF_STRUCT_OPS(simple_enable, struct task_struct *p) {}

void BPF_STRUCT_OPS(simple_disable, struct task_struct *p) {}

void BPF_STRUCT_OPS(simple_cpu_online, s32 cpu) {}

void BPF_STRUCT_OPS(simple_cpu_offline, s32 cpu) {}

s32 BPF_STRUCT_OPS_SLEEPABLE(simple_init)
{
	return scx_bpf_create_dsq(SCX_DSQ_GLOBAL, -1);
}

void BPF_STRUCT_OPS(simple_exit, struct scx_exit_info *info) {}

SEC(".struct_ops.link")
struct sched_ext_ops simple_ops = {
	.select_cpu = (void *)simple_select_cpu,
	.enqueue = (void *)simple_enqueue,
	.dequeue = (void *)simple_dequeue,
	.dispatch = (void *)simple_dispatch,
	.tick = (void *)simple_tick,
	.runnable = (void *)simple_runnable,
	.running = (void *)simple_running,
	.stopping = (void *)simple_stopping,
	.quiescent = (void *)simple_quiescent,
	.yield = (void *)simple_yield,
	.set_weight = (void *)simple_set_weight,
	.set_cpumask = (void *)simple_set_cpumask,
	.update_idle = (void *)simple_update_idle,
	.cpu_acquire = (void *)simple_cpu_acquire,
	.cpu_release = (void *)simple_cpu_release,
	.init_task = (void *)simple_init_task,
	.exit_task = (void *)simple_exit_task,
	.enable = (void *)simple_enable,
	.disable = (void *)simple_disable,
	.cpu_online = (void *)simple_cpu_online,
	.cpu_offline = (void *)simple_cpu_offline,
	.init = (void *)simple_init,
	.exit = (void *)simple_exit,
	.name = "simple",
};

// Made with Bob
