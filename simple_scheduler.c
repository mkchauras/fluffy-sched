/* SPDX-License-Identifier: GPL-2.0 */
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <bpf/libbpf.h>

static int running = 1;

static void stop(int sig)
{
	running = 0;
}

int main(void)
{
	struct bpf_object *obj;
	struct bpf_link *link;
	struct bpf_map *map;

	/* Suppress libbpf output */
	libbpf_set_print(NULL);

	/* Handle Ctrl+C */
	signal(SIGINT, stop);

	/* Open and load BPF program */
	obj = bpf_object__open_file("simple_scheduler.bpf.o", NULL);
	if (!obj)
		return 1;

	if (bpf_object__load(obj))
		return 1;

	/* Find and attach the scheduler */
	bpf_object__for_each_map(map, obj) {
		if (bpf_map__type(map) == BPF_MAP_TYPE_STRUCT_OPS) {
			link = bpf_map__attach_struct_ops(map);
			if (link)
				break;
		}
	}

	if (!link)
		return 1;

	printf("Scheduler running. Press Ctrl+C to exit.\n");

	/* Wait until Ctrl+C */
	while (running)
		sleep(1);

	/* Cleanup */
	bpf_link__destroy(link);
	bpf_object__close(obj);
	return 0;
}

// Made with Bob
