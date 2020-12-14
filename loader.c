/*
 * Loader Implementation
 *
 * 2018, Operating Systems
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/mman.h>
#include "exec_parser.h"
static struct sigaction old_action;
static so_exec_t *exec;
static int file_descriptor;
static int pagesize;
static void *find(void *addr)
{
	int i, length, indexs, index;
	void *start, *end;

	length = exec->segments_no;
	for (i = 0; i < length; i++) {
		indexs = exec->segments[i].mem_size / pagesize;
		start = (void *)exec->segments[i].vaddr;
		end = (void *)(start + exec->segments[i].mem_size);
		if (start <= addr && addr < end) {
			index = (int)(addr - start) / pagesize;
			if (!exec->segments[i].data) {
				exec->segments[i].data = calloc(indexs, sizeof(unsigned char));
				return exec->segments + i;
			} else if (((unsigned char *)exec->segments[i].data)[index] == 0) {
				return exec->segments + i;
			} else {
				goto exit_find;
			}
		}
	}
exit_find:
	return NULL;
}
static void read_(void *buffer, int length, int where)
{
	int buffer_read;

	buffer_read = 0;
	lseek(file_descriptor, where, SEEK_SET);
	while (buffer_read < length)
		buffer_read += read(file_descriptor,
		buffer + buffer_read, length - buffer_read);
}
static void sig_handler(int signal, siginfo_t *siginfo, void *c)
{

	int read_length, distance;
	so_seg_t *segment;
	void *page_address;

	if (signal != SIGSEGV)
		goto exit_err;
	segment = find(siginfo->si_addr);
	if (!segment)
		goto exit_err;
	distance = (((uintptr_t)siginfo->si_addr - segment->vaddr)
	/ pagesize) * pagesize;
	page_address = (void *)(segment->vaddr + distance);
	mmap(page_address, pagesize, PROT_WRITE,
		MAP_ANONYMOUS | MAP_FIXED | MAP_PRIVATE, -1, 0);
	read_length = segment->file_size - distance;
	read_length = read_length < pagesize ? read_length : pagesize;
	if (read_length > 0)
		read_(page_address, read_length, segment->offset + distance);
	if (read_length < pagesize)
		memset(page_address + read_length, 0, pagesize - read_length);
	mprotect(page_address, pagesize, segment->perm);
	((unsigned char *)segment->data)[distance / pagesize] = 1;
	goto exit;
exit_err:
		old_action.sa_sigaction(signal, siginfo, c);
exit:
		return;
}


int so_init_loader(void)
{
	struct sigaction sig;

	pagesize = getpagesize();
	sig.sa_sigaction = sig_handler;
	sig.sa_flags = SA_SIGINFO;
	sigaction(SIGSEGV, &sig, &old_action);
	return -1;
}

int so_execute(char *path, char *argv[])
{
	file_descriptor = open(path, O_RDONLY);

	if (file_descriptor < 0)
		return -1;
	exec = so_parse_exec(path);
	if (!exec)
		return -1;

	so_start_exec(exec, argv);

	return -1;
}
