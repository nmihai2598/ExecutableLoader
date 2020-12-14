#include "so_scheduler.h"

static Sch sch;

int compare_io(void *first, void *secound)
{
	Thr x;
	unsigned int io;

	x = (Thr) first;
	io = *(unsigned int *) secound;
	if (x->wait_io == io)
		return 1;
	else
		return 0;
}
int compare_priority(void *first, void *secound)
{
	Thr x, y;

	x = (Thr) first;
	y = (Thr) secound;
	if (x->priority < y->priority)
		return 1;
	else
		return 0;
}

void Scheduler_action(void)
{
	Thr current;

	current = sch->Running;
	if (!current->quant) {
		current->quant = sch->quant;
		add_queue(sch->Ready, current, sch->compare_priority);
		sch->Running = get_queue(sch->Ready);
	}
	if (current != sch->Running) {
		sem_post(&sch->Running->wait);
		sem_wait(&current->wait);
	}
}

int so_init(unsigned int time_quantum, unsigned int io)
{
	if (io > 256 || sch || time_quantum == 0)
		goto exit;
	sch = calloc(1, sizeof(Scheduler));
	if (sch == NULL)
		goto exit;
	sch->Ready = calloc(1, sizeof(TCoada));
	if (sch->Ready == NULL)
		goto exit1;
	sch->compare_io = compare_io;
	sch->compare_priority = compare_priority;
	sch->quant = time_quantum;
	sch->no_io = io;
	return 0;
exit1:
	free(sch);
exit:
	return -1;
}

void *thread_function(void *args)
{
	Thr thread;

	thread = (Thr) args;
	sem_wait(&thread->wait);
	thread->func(thread->priority);
	sch->Running = get_queue(sch->Ready);
	if (sch->Running)
		sem_post(&sch->Running->wait);
	return NULL;
}

tid_t so_fork(so_handler *func, unsigned int priority)
{
	Thr thread, current;

	if (!func || priority > 5)
		return 0;
	thread = calloc(1, sizeof(Thread));
	if (!thread)
		return 0;

	if (sem_init(&thread->wait, 0, 0))
		goto error;
	thread->func = func;
	thread->priority = priority;
	thread->quant = sch->quant;
	if (pthread_create(&thread->id, NULL,
			thread_function, (void *) thread))
		goto error1;
	add_list(&sch->all_thread, thread);
	if (!sch->Running) {
		sch->Running = thread;
		sem_post(&sch->Running->wait);
		return thread->id;
	}
	if (sch->Running->priority < thread->priority) {
		current = sch->Running;
		add_queue(sch->Ready, sch->Running, sch->compare_priority);
		sch->Running = thread;
		sem_post(&sch->Running->wait);
		current->quant = sch->quant;
		sem_wait(&current->wait);
		return thread->id;
	}
	add_queue(sch->Ready, thread, sch->compare_priority);
	sch->Running->quant -= 1;
	Scheduler_action();
	return thread->id;

error1:
	sem_destroy(&thread->wait);
error:
	free(thread);
	return 0;
}
void so_exec(void)
{
	sch->Running->quant -= 1;
	Scheduler_action();
}
int so_wait(unsigned int io)
{
	Thr current;

	if (io >= sch->no_io) {
		sch->Running->quant -= 1;
		Scheduler_action();
		return -1;
	}
	current = sch->Running;
	current->wait_io = io;
	current->quant = sch->quant;
	add_list(&sch->waiting, current);
	sch->Running = get_queue(sch->Ready);
	sem_post(&sch->Running->wait);
	sem_wait(&current->wait);
	return 0;
}
int so_signal(unsigned int io)
{
	Thr next, current;
	TL list;
	int count = 0;

	if (io >= sch->no_io)
		return -1;
	list = get_list(&sch->waiting, &io, sch->compare_io, &count);
	if (!list)
		return 0;
	next = get_el_list(&list, sch->compare_priority);
	if (next->priority > sch->Running->priority) {
		current = sch->Running;
		current->quant = sch->quant;
		add_list_queue(sch->Ready, list, sch->compare_priority);
		add_queue(sch->Ready, current, sch->compare_priority);
		sch->Running = next;
		sem_post(&sch->Running->wait);
		sem_wait(&current->wait);
		return count;
	}
	add_list(&list, next);
	add_list_queue(sch->Ready, list, sch->compare_priority);
	sch->Running->quant -= 1;
	Scheduler_action();
	return count;
}
void so_end(void)
{
	TL threads, aux;
	Thr val;

	if (!sch)
		return;
	threads = sch->all_thread;
	while (threads) {
		val = (Thr)threads->info;
		pthread_join(val->id, NULL);
		sem_destroy(&val->wait);
		free(val);
		aux = threads;
		threads = threads->urm;
		free(aux);
	}
	free(sch->Ready);
	free(sch);
	sch = NULL;
}
