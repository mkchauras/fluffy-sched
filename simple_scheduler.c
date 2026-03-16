/* SPDX-License-Identifier: GPL-2.0 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <bpf/libbpf.h>
#include <bpf/bpf.h>

static volatile sig_atomic_t keep_running = 1;

static void sig_handler(int sig)
{
	keep_running = 0;
}

static int libbpf_print_fn(enum libbpf_print_level level, const char *format, va_list args)
{
	if (level == LIBBPF_WARN)
		return vfprintf(stderr, format, args);
	return 0;
}

int main(int argc, char **argv)
{
	struct bpf_object *obj;
	struct bpf_link *link = NULL;
	struct bpf_map *map;
	int err;

	libbpf_set_print(libbpf_print_fn);
	signal(SIGINT, sig_handler);
	signal(SIGTERM, sig_handler);

	obj = bpf_object__open_file("simple_scheduler.bpf.o", NULL);
	if (!obj) {
		fprintf(stderr, "Failed to open BPF object\n");
		return 1;
	}

	err = bpf_object__load(obj);
	if (err) {
		fprintf(stderr, "Failed to load BPF object: %d\n", err);
		goto cleanup;
	}

	bpf_object__for_each_map(map, obj) {
		if (bpf_map__type(map) == BPF_MAP_TYPE_STRUCT_OPS) {
			link = bpf_map__attach_struct_ops(map);
			if (!link) {
				err = -errno;
				fprintf(stderr, "Failed to attach: %s\n", strerror(errno));
				goto cleanup;
			}
			printf("Scheduler attached. Press Ctrl+C to exit.\n");
			break;
		}
	}

	if (!link) {
		fprintf(stderr, "No struct_ops map found\n");
		err = -1;
		goto cleanup;
	}

	while (keep_running)
		sleep(1);

cleanup:
	if (link)
		bpf_link__destroy(link);
	bpf_object__close(obj);
	return err != 0;
}

// Made with Bob
