/* SPDX-License-Identifier: GPL-2.0 */
#define BPF_NO_KFUNC_PROTOTYPES
#include "/usr/src/kernels/6.18.13-200.fc43.x86_64/vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>

char _license[] SEC("license") = "GPL";

#define SCX_SLICE_DFL (20ULL * 1000000)
#define SCX_DSQ_GLOBAL 0

void scx_bpf_dsq_insert(struct task_struct *p, u64 dsq_id, u64 slice, u64 enq_flags) __ksym;
bool scx_bpf_dsq_move_to_local(u64 dsq_id) __ksym;
s32 scx_bpf_create_dsq(u64 dsq_id, s32 node) __ksym;

#define BPF_STRUCT_OPS(name, args...) \
SEC("struct_ops/"#name) \
BPF_PROG(name, ##args)

#define BPF_STRUCT_OPS_SLEEPABLE(name, args...) \
SEC("struct_ops.s/"#name) \
BPF_PROG(name, ##args)

s32 BPF_STRUCT_OPS(simple_enqueue, struct task_struct *p, u64 enq_flags)
{
	scx_bpf_dsq_insert(p, SCX_DSQ_GLOBAL, SCX_SLICE_DFL, 0);
	return 0;
}

void BPF_STRUCT_OPS(simple_dispatch, s32 cpu, struct task_struct *prev)
{
	scx_bpf_dsq_move_to_local(SCX_DSQ_GLOBAL);
}

s32 BPF_STRUCT_OPS_SLEEPABLE(simple_init)
{
	return scx_bpf_create_dsq(SCX_DSQ_GLOBAL, -1);
}

void BPF_STRUCT_OPS(simple_exit, struct scx_exit_info *info) {}

SEC(".struct_ops.link")
struct sched_ext_ops simple_ops = {
	.enqueue = (void *)simple_enqueue,
	.dispatch = (void *)simple_dispatch,
	.init = (void *)simple_init,
	.exit = (void *)simple_exit,
	.name = "simple",
};
