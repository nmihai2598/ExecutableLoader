#ifndef UTILS
#include <stdio.h>
#include <stdlib.h>
#define UTILS
#define TOP(a) (((AQ)(a))->top)
typedef int (so_compare)(void *, void *);

typedef struct list {
	void *info;
	struct list *urm;
} Tlist, *TL;

typedef struct coada {
	TL top, end;
} TCoada, *AQ;

void add_list(TL *list, void *info);
void *get_el_list(TL *list, so_compare *compare);
void *get_list(TL *list, void *io, so_compare *compare, int *count);
void *get_queue(AQ queue);
void add_queue(AQ queue, void *info, so_compare *compare);
void add_list_queue(AQ queue, TL list, so_compare *compare);
#endif
